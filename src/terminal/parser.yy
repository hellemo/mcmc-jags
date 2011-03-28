/* -*-C++-*- */
%{
#include <config.h>

#ifdef Win32
#include <windows.h>   /* For getCurrentDirectory */
#include <io.h>        /* For chdir */
#else
#include <unistd.h>    /* For getcwd, chdir */
#endif

//#include <limits.h>

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <fstream>
#include <list>
#include <iterator>
#include <string>
#include <utility>

#include <dirent.h>
#include <time.h>
#include <errno.h>

#include <Console.h>
#include <Module.h>
#include <compiler/ParseTree.h>
#include <util/nainf.h>
#include <cstring>
#include <ltdl.h>

//Required for warning about masked distributions after module loading
#include <deque>
#include <distribution/Distribution.h>
#include <compiler/Compiler.h>

#include "ReadData.h"

    typedef void(*pt2Func)();

    int zzerror(const char *);
    int zzlex();
    int zzlex_destroy();
#define YYERROR_VERBOSE 0
    static Console *console;
    bool interactive;
    extern int command_buffer_count;
    void setName(ParseTree *p, std::string *name);
    std::map<std::string, SArray> _data_table;
    std::deque<lt_dlhandle> _dyn_lib;
    bool open_data_buffer(std::string const *name);
    bool open_command_buffer(std::string const *name);
    void return_to_main_buffer();
    void setMonitor(ParseTree const *var, int thin, std::string const &type);
    void clearMonitor(ParseTree const *var, std::string const &type);
    void doCoda (ParseTree const *var, std::string const &stem);
    void doAllCoda (std::string const &stem);
    void doDump (std::string const &file, DumpType type, unsigned int chain);
    void dumpMonitors(std::string const &file, std::string const &type);
    void doSystem(std::string const *command);
    std::string ExpandFileName(char const *s);

    static bool getWorkingDirectory(std::string &name);
    static void errordump();
    static void updatestar(long niter, long refresh, int width);
    static void adaptstar(long niter, long refresh, int width);
    static void setParameters(ParseTree *p, ParseTree *param1);
    static void setParameters(ParseTree *p, std::vector<ParseTree*> *parameters);
    static void setParameters(ParseTree *p, ParseTree *param1, ParseTree *param2);
    static void loadModule(std::string const &name);
    static void unloadModule(std::string const &name);
    static void dumpSamplers(std::string const &file);
    static void delete_pvec(std::vector<ParseTree*> *);
    static void print_unused_variables();
    static void listFactories(FactoryType type);
    static void setFactory(std::string const &name, FactoryType type,
                           std::string const &status);

    %}

%defines
%name-prefix="zz"
%union
{
  int intval;
  double val;
  std::string *stringptr;
  ParseTree *ptree;
  std::vector<ParseTree*> *pvec;
  std::vector<double> *vec;
  std::vector<long> *ivec;
}

%token <intval> INT
%token <val>    DOUBLE
%token <stringptr> NAME
%token <stringptr> STRING
%token <stringptr> SYSCMD
%token <intval> ENDCMD

%token <intval> MODEL
%token <intval> DATA
%token <intval> IN 
%token <intval> TO
%token <intval> INITS
%token <intval> PARAMETERS

%token <intval> COMPILE
%token <intval> INITIALIZE
%token <intval> ADAPT
%token <intval> UPDATE
%token <intval> BY
%token <intval> MONITORS
%token <intval> MONITOR
%token <intval> TYPE
%token <intval> SET
%token <intval> CLEAR
%token <intval> THIN
%token <intval> CODA
%token <intval> STEM
%token <intval> EXIT
%token <intval> NCHAINS
%token <intval> CHAIN
%token <intval> LOAD
%token <intval> UNLOAD
%token <intval> SAMPLER
%token <intval> SAMPLERS
%token <intval> RNGTOK
%token <intval> FACTORY;
%token <intval> FACTORIES;

%token <intval> LIST 
%token <intval> STRUCTURE
%token <intval> DIM
%token <intval> NA
%token <intval> R_NULL
%token <intval> DIMNAMES
%token <intval> ITER
%token <intval> ARROW
%token <intval> ENDDATA
%token <intval> ASINTEGER

%token <intval> DIRECTORY
%token <intval> CD
%token <intval> PWD
%token <intval> RUN
%token <intval> ENDSCRIPT

%type <ptree> var index 
%type <ptree> r_assignment r_structure
%type <ptree> range_element r_dim 
%type <ptree> r_attribute_list
%type <ptree> r_value 
%type <pvec>  r_value_list r_assignment_list range_list
%type <ptree> r_value_collection r_integer_collection r_collection
%type <stringptr> file_name;
%type <stringptr> r_name;

%%

input: {
    if (interactive && command_buffer_count == 0) 
	std::cout << ". " << std::flush;
}
| input line {
    if (interactive && command_buffer_count == 0) 
	std::cout << ". " << std::flush;
}
;

line: ENDCMD {}
| command ENDCMD {}
| error ENDCMD {if(interactive) yyerrok; else exit(1); }
| run_script {}
| ENDSCRIPT ENDCMD {}
| SYSCMD ENDCMD { doSystem($1); delete $1;}
;

command: model 
| data_in
| data_to
| data_clear
| parameters_in
| parameters_to
| compile
| initialize
| adapt
| update
| monitor
| monitors_to
| coda
| load
| unload
| exit
| read_dir
| get_working_dir
| set_working_dir
| samplers_to
| list_factories
| set_factory
;

model: MODEL IN file_name {
    std::FILE *file = std::fopen(ExpandFileName(($3)->c_str()).c_str(), "r");
    if (!file) {
	std::cerr << "Failed to open file " << *($3) << std::endl;
    }
    else {
	console->checkModel(file);
	std::fclose(file);
    }
    delete $3;
}
| MODEL CLEAR {
    console->clearModel();
}
;

data_in: data r_assignment_list ENDDATA {
    std::string rngname;
    readRData($2, _data_table, rngname);
    if (rngname.size() != 0) {
	std::cerr << "WARNING: .RNG.name assignment ignored" << std::endl;
    }
    delete_pvec($2);
 }
| data {}    // Failed to open the data file 
;

data_to: DATA TO file_name {
    doDump(*$3, DUMP_DATA, 1);
    delete $3;
}
;

data: DATA IN file_name {
    if(open_data_buffer($3)) {
	std::cout << "Reading data file " << *$3 << std::endl;
    }
    else {
	std::cerr << "Unable to open file " << *$3 << std::endl << std::flush;
    }
    delete $3;
 }
;

data_clear: DATA CLEAR {
  std::cout << "Clearing data table " << std::endl;
  _data_table.clear();
}
;

parameters_in: parameters r_assignment_list ENDDATA
{
    std::map<std::string, SArray> parameter_table;
    std::string rngname;
    readRData($2, parameter_table, rngname);
    delete_pvec($2);
    /* Set all chains to the same state. If the user sets the
       RNG state in addition to the parameter values then all
       chains will be identical!
    */
    if (console->model() == 0) {
	std::cout << "WARNING: Initial values ignored. "
		  <<  "(You must compile the model first)" << std::endl;
    }
    for (unsigned int i = 1; i <= console->nchain(); ++i) {
	/* We have to set the name first, because the state or seed
	   might be embedded in the parameter_table */
	if (rngname.size() != 0) {
	    console->setRNGname(rngname, i);
	}
	console->setParameters(parameter_table, i);
    }
}
| parameters r_assignment_list ENDDATA ',' CHAIN '(' INT ')' 
{
    std::map<std::string, SArray> parameter_table;
    std::string rngname;
    readRData($2, parameter_table, rngname);
    delete $2;
    /* We have to set the name first, because the state or seed
       might be embedded in the parameter_table */
    if (rngname.size() != 0) {
        console->setRNGname(rngname, $7);
    }
    console->setParameters(parameter_table, $7);
}
| parameters {} // Failed to open the file
;

parameters_to: PARAMETERS TO file_name {
    doDump(*$3, DUMP_PARAMETERS, 1);
    delete $3;
}
| PARAMETERS TO file_name ',' CHAIN '(' INT ')' {
    doDump(*$3, DUMP_PARAMETERS, $7);
    delete $3;
}
;

parameters: PARAMETERS IN file_name {
  if(open_data_buffer($3)) {
    std::cout << "Reading parameter file " << *$3 << std::endl;
  }
  else {
    std::cerr << "Unable to open file " << *$3 << std::endl << std::flush;
  }
  delete $3;
}
| INITS IN file_name {
  /* Legacy option to not break existing scripts */
  if(open_data_buffer($3)) {
    std::cout << "Reading initial values file " << *$3 << std::endl;
  }
  else {
    std::cerr << "Unable to open file " << *$3 << std::endl << std::flush;
  }
  delete $3;
}
;

compile: COMPILE {
    console->compile(_data_table, 1, true);
    print_unused_variables();
}
| COMPILE ',' NCHAINS '(' INT ')' {
    console->compile(_data_table, $5, true);
    print_unused_variables();
}
;

initialize: INITIALIZE {
  if (!console->initialize()) {
    errordump();
  }
}
;

adapt: ADAPT INT {
    long refresh = $2/40;
    adaptstar($2, refresh, 40);
}
| ADAPT INT ',' BY '(' INT ')' {
    adaptstar($2,$6,40);
}
;

update: UPDATE INT {
  //fixme: put refresh option
  long refresh = $2/40;
  updatestar($2, refresh, 40);
}
| UPDATE INT ',' BY '(' INT ')' {
  updatestar($2,$6, 40);
}
;

exit: EXIT { return 0; }
;

var: NAME {
  $$ = new ParseTree(P_VAR); setName($$, $1);
}
| NAME '[' range_list ']' {
  $$ = new ParseTree(P_VAR); setName($$, $1);
  setParameters($$, $3);
}
;

range_list: range_element {
  $$ = new std::vector<ParseTree*>(1, $1); 
}
| range_list ',' range_element {
  $$=$1; $$->push_back($3);
}
;

range_element: index {
  $$ = new ParseTree(P_RANGE); setParameters($$,$1);
}
| index ':' index {
  $$ = new ParseTree(P_RANGE); setParameters($$, $1, $3);
}
;

/* FIXME: Use integer value here */
index: INT {$$ = new ParseTree(P_VALUE); $$->setValue($1);}
;

monitor: monitor_set
| monitor_clear
;

monitor_set: MONITOR SET var  { 
    setMonitor($3, 1, "trace"); delete $3;
}
| MONITOR SET var ',' THIN '(' INT ')' { 
    setMonitor($3, $7, "trace"); delete $3;
}
| MONITOR var {
    setMonitor($2, 1, "trace"); delete $2;
}
| MONITOR var ',' THIN '(' INT ')' { 
    setMonitor($2, $6, "trace"); delete $2;
}
| MONITOR var ',' TYPE '(' NAME ')' {
    setMonitor($2, 1, *$6);
    delete $6;
}
| MONITOR var ',' TYPE '(' NAME ')' THIN '(' INT ')' {
    setMonitor($2, $10, *$6); 
    delete $6;
}
| MONITOR var ',' THIN '(' INT ')' TYPE '(' NAME ')' {
    setMonitor($2, $6, *$10); 
    delete $10;
}
;

monitor_clear: MONITOR CLEAR var {
    clearMonitor($3, "trace"); delete $3;
}
| MONITOR CLEAR var ',' TYPE '(' NAME ')' {
    clearMonitor($3, *$7);
    delete $7;
}
;

monitors_to:  MONITORS TO file_name 
{
    dumpMonitors(*$3, "trace");
    delete $3;
}
|
MONITORS TO file_name ',' TYPE '(' NAME ')' {
    dumpMonitors(*$3, *$7);
    delete $3;
    delete $7; 
}
;


/* 
   File names may optionally be enclosed in quotes, and this is required
   if the name includes spaces.
*/
file_name: NAME { $$ = $1;}
| STRING { $$ = $1; }
;

coda: CODA var {
  doCoda ($2, "CODA"); delete $2;
}
| CODA var ',' STEM '(' file_name ')' {
  doCoda ($2, *$6); delete $2; delete $6;
}
| CODA '*' {
  doAllCoda ("CODA"); 
}
| CODA '*' ',' STEM '(' file_name ')' {
  doAllCoda (*$6); delete $6; 
}
;

load: LOAD file_name { loadModule(*$2); }
;

unload: UNLOAD NAME { unloadModule(*$2); }
;

samplers_to: SAMPLERS TO file_name 
{
    dumpSamplers(*$3);
    delete $3;
}
;

list_factories: LIST FACTORIES ',' TYPE '(' SAMPLER ')'
{
    listFactories(SAMPLER_FACTORY);
}
|
LIST FACTORIES ',' TYPE '(' RNGTOK ')'
{
    listFactories(RNG_FACTORY);
}
|
LIST FACTORIES ',' TYPE '(' MONITOR ')'
{
    listFactories(MONITOR_FACTORY);
}
;

set_factory: SET FACTORY STRING NAME ',' TYPE '(' SAMPLER ')'
{
    setFactory(*$3, SAMPLER_FACTORY, *$4);
    delete $3;
    delete $4;
}
|
SET FACTORY NAME NAME ',' TYPE '(' RNGTOK ')'
{
    setFactory(*$3, RNG_FACTORY, *$4);
    delete $3;
    delete $4;
}
|
SET FACTORY NAME NAME ',' TYPE '(' MONITOR ')'
{
    setFactory(*$3, MONITOR_FACTORY, *$4);
    delete $3;
    delete $4;
}
;

/* Rules for scanning dumped R datasets */

r_assignment_list: r_assignment {
  $$ = new std::vector<ParseTree*>(1, $1);
}
| r_assignment_list r_assignment {
  $$ = $1; $$->push_back($2);
}
| r_assignment_list ';' r_assignment {
  $$ = $1; $$->push_back($3);
}
;

r_assignment: r_name ARROW r_structure {
  $$ = $3; setName($$, $1);
}
| r_name ARROW r_collection {
  $$ = new ParseTree(P_ARRAY);
  setName($$, $1);
  setParameters($$, $3);
}
| r_name ARROW STRING {
  /* Allow this for setting the NAME of the random number generator */
  $$ = new ParseTree(P_VAR); setName($$, $1);
  ParseTree *p = new ParseTree(P_VAR); setName(p, $3);
  setParameters($$, p);
}
;

r_name: STRING
| NAME
| '`' NAME '`' {
    /* R >= 2.4.0 uses backticks for quoted names */
    $$ = $2;
}

r_structure: STRUCTURE '(' r_collection ',' r_attribute_list ')' {
  $$ = new ParseTree(P_ARRAY); 
  if ($5) 
    setParameters($$, $3, $5);
  else
    setParameters($$, $3);
}
;

/* The only attribute we are interested in is .Dim. The rest are
   simply discarded - see below */
r_attribute_list: r_dim
| r_generic_attribute {$$=0;}
| r_attribute_list ',' r_generic_attribute
| r_attribute_list ',' r_dim {$$=$3;}
;

r_dim: DIM '=' r_collection {
  $$ = $3;
}
;

r_collection: r_integer_collection
| r_value_collection
;

r_integer_collection: ASINTEGER '(' r_value_collection ')' {$$ = $3;}
;

r_value_collection: r_value { 
  $$ = new ParseTree(P_VECTOR); 
  setParameters($$, $1);
}
| 'c' '(' r_value_list ')' {
  $$ = new ParseTree(P_VECTOR);
  setParameters($$, $3);
}
;
 
r_value_list: r_value {$$ = new std::vector<ParseTree*>(1, $1); }
| r_value_list ',' r_value {$$ = $1; $$->push_back($3);}
;

r_value: DOUBLE {$$ = new ParseTree(P_VALUE); $$->setValue($1);}
| NA {$$ = new ParseTree(P_VALUE); $$->setValue(JAGS_NA);}
;

/* Rules for parsing generic attributes.  We don't want to do anything
   with the results, just have the parser accept them */

r_generic_attribute: NAME '=' r_generic_vector {;}
;

r_generic_list: r_generic_list_element {;}
| r_generic_list ',' r_generic_list_element {;}
;

r_generic_list_element: r_generic_vector {;}
| NAME '=' r_generic_vector {;}
;

r_generic_vector: r_numeric_vector {;}
| ASINTEGER '(' r_numeric_vector ')' {;}
| r_character_vector {;}
| LIST '(' r_generic_list ')' {;}
| STRUCTURE '(' r_generic_list ')' {;}
| R_NULL {;}
;

r_numeric_vector: DOUBLE {;}
| 'c' '(' r_double_list ')'
;

r_double_list: DOUBLE {;}
| r_double_list ',' DOUBLE {;}
;

r_character_vector: STRING {;}
| 'c' '(' r_string_list ')' {;}
;

r_string_list: STRING {;}
| r_string_list ',' STRING {;}
;

/* Rules for interacting with the operating system */

get_working_dir: PWD
{
    std::string name;
    if (getWorkingDirectory(name)) {
	std::cout << name << std::endl;
    }
    else {
	std::cout << "ERROR: " << name << std::endl;
    }
}

set_working_dir: CD file_name
{
    if (chdir(($2)->c_str()) == -1) {
	std::cout << "ERROR: Cannot change working directory" << std::endl;
    }
    delete $2;
}

read_dir: DIRECTORY
{
    std::string name;
    if (!getWorkingDirectory(name)) {
	std::cerr << "ERROR: Unable to get working directory name\n"
		  << name << std::endl;
	return 0;
    }
	
    DIR *dir;
    struct dirent *de;
    if ((dir = opendir(name.c_str())) != 0) {
	while ((de = readdir(dir)) != 0) {
	    if (std::strcmp(de->d_name, ".") && std::strcmp(de->d_name, "..")) {
		std::cout << de->d_name << "\n";
	    }
	}
	closedir(dir);
    }
    else {
	std::cerr << "Unable to open working directory" << std::endl;
    }
}    

run_script: RUN file_name {
    if(open_command_buffer($2)) {
	std::cout << "Running script file " << *$2 << std::endl;
    }
    else {
	std::cerr << "Unable to open script file " << *$2 << std::endl;
    }
    delete $2;
 }
;

%%

int zzerror (const char *s)
{
    return_to_main_buffer();
    std::cerr << s << std::endl;
    return 0;
}

static Range getRange(ParseTree const *var)
{
  /* 
     Blank arguments, e.g. foo[] or bar[,1]  are not allowed.
  */
  unsigned int size = var->parameters().size();

  std::vector<int>  ind_lower(size), ind_upper(size);
  for (unsigned int i = 0; i < size; ++i) {
    ParseTree const *range_element = var->parameters()[i];
    switch(range_element->parameters().size()) {
    case 1:
      ind_lower[i] = (int) (range_element->parameters()[0]->value() + 1.0E-6);
      ind_upper[i] = ind_lower[i];
      break;
    case 2:
      ind_lower[i] = (int) (range_element->parameters()[0]->value() + 1.0E-6);  
      ind_upper[i] = (int) (range_element->parameters()[1]->value() + 1.0E-6);
      break;
    default:
      //Error! FIXME
      break;
    }
  }
  return Range(ind_lower, ind_upper);
}

void setMonitor(ParseTree const *var, int thin, std::string const &type)
{
    std::string const &name = var->name();
    if (var->parameters().empty()) {
	/* Requesting the whole node */
	console->setMonitor(name, Range(), thin, type);
    }
    else {
	/* Requesting subset of a multivariate node */
	console->setMonitor(name, getRange(var), thin, type);
    }
}

void clearMonitor(ParseTree const *var, std::string const &type)
{
    std::string const &name = var->name();
    if (var->parameters().empty()) {
	/* Requesting the whole node */
	console->clearMonitor(name, Range(), type);
    }
    else {
	/* Requesting subset of a multivariate node */
	console->clearMonitor(name, getRange(var), type);
    }
}

void doAllCoda (std::string const &stem)
{
    console->coda(stem);
}

void doCoda (ParseTree const *var, std::string const &stem)
{
    //FIXME: Allow list of several nodes

    std::vector<std::pair<std::string, Range> > dmp;
    if (var->parameters().empty()) {
	/* Requesting the whole node */
	dmp.push_back(std::pair<std::string,Range>(var->name(), Range()));
    }
    else {
	/* Requesting subset of a multivariate node */
	dmp.push_back(std::pair<std::string,Range>(var->name(), getRange(var)));
    }
    console->coda(dmp, stem);
}

/* Helper function for doDump that handles all the special cases
   (missing values etc) when writing a double value */
static void writeValue(double x, std::ostream &out, bool isdiscrete)
{
  using namespace std;

  if (x == JAGS_NA) {
    out << "NA";
  }
  else if (jags_isnan(x)) {
    out << "NaN";
  }
  else if (!jags_finite(x)) {
    if (x > 0)
      out << "Inf";
    else
      out << "-Inf";
  }
  else if (isdiscrete) {
      out << static_cast<int>(x) << "L";
  }
  else {
    out << x;
  }
}

void doDump(std::string const &file, DumpType type, unsigned int chain)
{
    std::map<std::string,SArray> data_table;
    std::string rng_name;
    if (!console->dumpState(data_table, rng_name, type, chain)) {
	return;
    }

    /* Open output file */
    std::ofstream out(file.c_str());
    if (!out) {
	std::cerr << "Failed to open file " << file << std::endl;
	return;
    }
  
    if (rng_name.size() != 0) {
	out << "`.RNG.name` <- \"" << rng_name << "\"\n";
    }

    for (std::map<std::string, SArray>::const_iterator p = data_table.begin();
	 p != data_table.end(); ++p) {
	std::string const &name = p->first;
	SArray const &sarray = p->second;
	std::vector<double> const &value = sarray.value();
	long length = sarray.length();
	out << "`" << name << "` <- " << std::endl;
	std::vector<unsigned int> const &dim = sarray.dim(false);
	bool discrete = sarray.isDiscreteValued();

	if (dim.size() == 1) {
	    // Vector 
	    if (dim[0] == 1) {
		// Scalar
		writeValue(value[0], out, discrete);
	    }
	    else {
		// Vector of length > 1
		out << "c(";
		for (int i = 0; i < length; ++i) {
		    if (i > 0) {
			out << ",";
		    }
		    writeValue(value[i], out, discrete);
		}
		out << ")";
	    }
	}
	else {
	    // Array 
	    out << "structure(c(";
	    for (int i = 0; i < length; ++i) {
		if (i > 0) {
		    out << ",";
		}
		writeValue(value[i], out, discrete);
	    }
	    out << "), .Dim = c(";
	    for (unsigned int j = 0; j < dim.size(); ++j) {
		if (j > 0) {
		    out << ",";
		}
		out << dim[j] << "L";
	    }
	    out << "))";
	}
	out << "\n";
    }
    out.close();
}  

void dumpMonitors(std::string const &file, std::string const &type)
{
    std::map<std::string,SArray> data_table;

    if (!console->dumpMonitors(data_table, type, false)) {
	return;
    }

    /* Open output file */
    std::ofstream out(file.c_str());
    if (!out) {
	std::cerr << "Failed to open file " << file << std::endl;
	return;
    }

    out << "`" << type << "` <-\nstructure(list(";

    std::map<std::string, SArray>::const_iterator p;
    for (p = data_table.begin(); p != data_table.end(); ++p) {
	std::string const &name = p->first;
	SArray const &sarray = p->second;
	std::vector<double> const &value = sarray.value();
	long length = sarray.length();

	if (p != data_table.begin()) {
	    out << ", \n";
	}
	out << "\"" << name << "\" = ";
	std::vector<unsigned int> const &dim = sarray.dim(false);
	bool discrete = sarray.isDiscreteValued();
	bool named = !sarray.dimNames().empty();

	if (dim.size() == 1 && !named) {
	    // Vector 
	    if (dim[0] == 1) {
		// Scalar
		writeValue(value[0], out, discrete);
	    }
	    else {
		// Vector of length > 1
		out << "c(";
		for (int i = 0; i < length; ++i) {
		    if (i > 0) {
			out << ",";
		    }
		    writeValue(value[i], out, discrete);
		}
		out << ")";
	    }
	}
	else {
	    // Array 
	    out << "structure(c(";
	    for (int i = 0; i < length; ++i) {
		if (i > 0) {
		    out << ",";
		}
		writeValue(value[i], out, discrete);
	    }
	    out << "), .Dim = ";
	    if (named) {
		out << "structure(";
	    }
	    out << "c(";
	    for (unsigned int j = 0; j < dim.size(); ++j) {
		if (j > 0) {
		    out << ",";
		}
		out << dim[j] << "L";
	    }
	    out << ")";
	    if (named) {
		std::vector<std::string> const &dnames = sarray.dimNames();
		out << ", .Names = c(";
		for (unsigned int k = 0; k < dnames.size(); ++k) {
		    if (k > 0) {
			out << ",";
		    }
		    out << "\"" << dnames[k] << "\"";
		}
		out << "))";
	    }
	    out << ")";
	}
    }

    out << "), \n.Names = c(";
    for (p = data_table.begin(); p != data_table.end(); ++p) {
	if (p != data_table.begin()) {
	    out << ", ";
	}
	std::string const &name = p->first;
	out << "\"" << name << "\"";
    }
    out << "))";
    out.close();
}

void setParameters(ParseTree *p, std::vector<ParseTree*> *parameters)
{
  /* 
     The parser dynamically allocates vectors of (pointers to)
     parameters. These vectors must be deleted when we are done with
     them.
  */
  p->setParameters(*parameters);
  delete parameters; 
}

void setParameters(ParseTree *p, ParseTree *param1)
{
  /*
    Wrapper function that creates a vector containing param1
    to be passed to ParseTree::setParameters.
  */
  std::vector<ParseTree *> parameters(1, param1);
  p->setParameters(parameters);
}

void setParameters(ParseTree *p, ParseTree *param1, ParseTree *param2)
{
  /*
    Wrapper function that creates a vector containing param1
    and param2, to be passed to ParseTree::setParameters
  */
  std::vector<ParseTree *> parameters;
  parameters.push_back(param1);
  parameters.push_back(param2);
  p->setParameters(parameters);
}

void setName(ParseTree *p, std::string *name)
{
  p->setName(*name);
  delete name;
}

static void errordump()
{
  if (console->model()) {
    std::ostringstream fname;
    for (unsigned int i = 1; i <= console->nchain(); ++i) {
       fname << "jags.dump" << i << ".R";
       std::cout << "Dumping chain " << i << " at iteration " 
                 << console->iter() << " to file " << fname.str() << std::endl;
       doDump(fname.str(), DUMP_ALL, i);
       fname.str("");
    }
  }
}

static void updatestar(long niter, long refresh, int width)
{
    bool adapt = console->isAdapting();
	
    if (refresh == 0) {
	console->update(niter/2);
	bool status = true;
	if (adapt && !console->adaptOff(status)) {
	    errordump();
	    return;
	}
	console->update(niter - niter/2);
	if (!status) {
	    std::cerr << "WARNING: Adaptation incomplete\n";
	}
	return;
    }

    if (width > niter / refresh + 1)
	width = niter / refresh + 1;

    std::cout << "Updating " << niter << std::endl;
    for (int i = 0; i < width - 1; ++i) {
	std::cout << "-";
    }
    std::cout << "| " << std::min(width * refresh, niter) << std::endl 
	      << std::flush;

    int col = 0;
    bool status = true;
    for (long n = niter; n > 0; n -= refresh) {
	if (adapt && n <= niter/2) {
	    // Turn off adaptive mode half way through burnin
	    if (console->adaptOff(status)) {
		adapt = false;
	    }
	    else {
		std::cout << std::endl;
		errordump();
		return;
	    }
	}
	long nupdate = std::min(n, refresh);
	if(console->update(nupdate))
	    std::cout << "*" << std::flush;
	else {
	    std::cout << std::endl;
	    errordump();
	    return;
	}
	col++;
	if (col == width || n <= nupdate) {
	    int percent = 100 - (n-nupdate) * 100/niter;
	    std::cout << " " << percent << "%" << std::endl;
	    if (n > nupdate) {
		col = 0;
	    }
	}
    }
    if (!status) {
	std::cerr << "WARNING: Adaptation incomplete\n";
    }
}

static void adaptstar(long niter, long refresh, int width)
{
    if (!console->isAdapting()) {
	std::cerr << "NOTE: Model is not in adaptive mode\n";
    }
	
    bool status = true;
    if (refresh == 0) {
	console->update(niter);
	if (!console->adaptOff(status)) {
	    errordump();
	    return;
	}
	if (!status) {
	    std::cerr << "WARNING: Adaptation incomplete\n";
	}
	return;
    }

    if (width > niter / refresh + 1)
	width = niter / refresh + 1;

    std::cout << "Adapting " << niter << std::endl;
    for (int i = 0; i < width - 1; ++i) {
	std::cout << "-";
    }
    std::cout << "| " << std::min(width * refresh, niter) << std::endl 
	      << std::flush;

    int col = 0;
    for (long n = niter; n > 0; n -= refresh) {
	long nupdate = std::min(n, refresh);
	if(console->update(nupdate))
	    std::cout << "+" << std::flush;
	else {
	    std::cout << std::endl;
	    errordump();
	    return;
	}
	col++;
	if (col == width || n <= nupdate) {
	    int percent = 100 - (n-nupdate) * 100/niter;
	    std::cout << " " << percent << "%" << std::endl;
	    if (n > nupdate) {
		col = 0;
	    }
	}
    }
    if (!console->adaptOff(status)) {
	std::cout << std::endl;
	errordump();
	return;
    }
    if (!status) {
	std::cerr << "WARNING: Adaptation incomplete\n";
    }
}

static void loadModule(std::string const &name)
{
    lt_dlhandle mod = lt_dlopenext(name.c_str());
    if (mod == NULL) {
	std::cout << lt_dlerror() << std::endl;
    }
    else {
	std::cout << "Loading module: " << name << std::endl;
	_dyn_lib.push_front(mod);
	Console::loadModule(name);
    }
}

static void unloadModule(std::string const &name)
{
    std::cout << "Unloading module: " << name << std::endl;
    Console::unloadModule(name);
}

int main (int argc, char **argv)
{
  extern std::FILE *zzin;

  std::FILE *cmdfile = 0;
  if (argc > 2) {
    std::cerr << "Too many arguments" << std::endl;
  }
  else if (argc == 2) {
    interactive = false;
    cmdfile = std::fopen(ExpandFileName(argv[1]).c_str(),"r");
    if (cmdfile) {
      zzin = cmdfile;
    }
    else {
      std::cerr << "Unable to open command file " << argv[1] << std::endl;
      return 1;
    }
  }
  else {
    interactive = true;
  }

#ifndef WIN32
  /* 
     - Allows emulation of dynamic loading on platforms that do not
     support it by preloading modules. 
     - Causes build failures on mingw-w64 (as at 21 April 2010) so
     not used on Windows platform.
  */
  LTDL_SET_PRELOADED_SYMBOLS();
#endif

  if(lt_dlinit()) {
      std::cerr << lt_dlerror() << std::endl;
      return 1;
  }

  /*
  pt2Func load_base = (pt2Func)(lt_dlsym(base, "load"));
  if (load_base == NULL) {
      std::cout << lt_dlerror() << std::endl;
      return 1;
  }
  else{
      (*load_base)();
  }
  */
  
  time_t t;
  time(&t);
  std::cout << "Welcome to " << PACKAGE_STRING << " on " << ctime(&t);
  std::cout << "JAGS is free software and comes with ABSOLUTELY NO WARRANTY" 
            << std::endl;
  loadModule("basemod");
  loadModule("bugs");

  console = new Console(std::cout, std::cerr);

  zzparse();
  zzlex_destroy();

  if (argc==2) {
      std::fclose(cmdfile);
  }
  
  //Unload modules
  std::vector<std::string> mods = Console::listModules();
  for (unsigned int i = 0; i < mods.size(); ++i) {
      Console::unloadModule(mods[i]);
  }
  delete console;
  //Release dynamic libraries. 
  for (unsigned int i = 0; i < _dyn_lib.size(); ++i) {
      lt_dlclose(_dyn_lib[i]);
  }
  lt_dlexit();
}

static bool getWorkingDirectory(std::string &name)
{
    char buf[FILENAME_MAX];
#ifdef Win32
    if (getCurrentDirectory(FILENAME_MAX, buf)) {
	name = buf;
	return true;
    }
    else {
	name = "Error in getCurrentDirectory";
	return false;
    }
#else
    if (getcwd(buf, FILENAME_MAX)) {
	name = buf;
	return true;
    }
    else {
	switch(errno) {
	case EACCES:
	    name = "Access denied";
	    break;
	case ENOENT:
	    name = "Not found";
	    break;
	case ERANGE:
	    name = "Directory name too long";
	    break;
	default:
	    name = "Error in getcwd";
	    break;
	}
	return false;
    }
#endif
}

static void dumpSamplers(std::string const &file)
{
    std::ofstream out(file.c_str());
    if (!out) {
	std::cerr << "Failed to open file " << file << std::endl;
	return;
    }

    std::vector<std::vector<std::string> > sampler_list;
    console->dumpSamplers(sampler_list);
    for (unsigned int i = 0; i < sampler_list.size(); ++i) {
	for (unsigned int j = 1; j < sampler_list[i].size(); ++j) {
	    out << i + 1 << "\t" 
		<< sampler_list[i][0] << "\t" //First element is sampler name
		<< sampler_list[i][j] << "\n"; //Rest are node names
	}
    }

    out.close();
}

static void delete_pvec(std::vector<ParseTree*> *pv)
{
    for (unsigned int i = 0; i < pv->size(); ++i) {
	delete (*pv)[i];
    }
    delete pv;
}

static void print_unused_variables()
{
    std::vector<std::string> data_vars;
    for (std::map<std::string, SArray>::const_iterator p = _data_table.begin();
	 p != _data_table.end(); ++p)
    {
	data_vars.push_back(p->first);
    }
    
    std::vector<std::string> unused_vars;
    std::vector<std::string> const &model_vars = console->variableNames();

    std::set_difference(data_vars.begin(), data_vars.end(),
			model_vars.begin(), model_vars.end(),
			std::inserter(unused_vars, unused_vars.begin()));

    if (!unused_vars.empty()) {
	std::cerr << "\nWARNING: Unused variable(s) in data table:\n";
	std::copy(unused_vars.begin(), unused_vars.end(),
		  std::ostream_iterator<std::string>(std::cerr, "\n"));
	std::cerr << "\n";
    }

}

std::string ExpandFileName(char const *s)
{
    if(s[0] != '~') return s;
    std::string name = s;
    if(name.size() > 1 && s[1] != '/') return s;

    char const *p = getenv("HOME");
    if (p) {
	std::string UserHOME = p;
	if (!UserHOME.empty()) {
	    if (name.size() == 1) 
		return UserHOME;
	    else
		return UserHOME + name.substr(1);
	}
    }
    return name;
}


void doSystem(std::string const *command)
{
    std::system(command->c_str());
}

void listFactories(FactoryType type)
{
    std::vector<std::pair<std::string, bool> > faclist = 
	Console::listFactories(type);

    std::vector<std::pair<std::string, bool> >::const_iterator p;
    unsigned int max_strlen = 0;
    for (p = faclist.begin(); p != faclist.end(); ++p) {
	if (p->first.length() > max_strlen)
	    max_strlen = p->first.length();
    }
    if (max_strlen < 4)
	max_strlen = 4;

    //Header
    std::cout << "Name";
    for (int i = max_strlen - 4; i >=0; --i) {
	std::cout << " ";
    }
    std::cout << "Status\n";

    //Body
    for (p = faclist.begin(); p != faclist.end(); ++p) {
	std::cout << p->first << " ";
	for (int i = max_strlen - p->first.length(); i >= 0; --i) {
	    std::cout << " ";
	}
	if (p->second) {
	    std::cout << "on";
	}
	else {
	    std::cout << "off";
	}
	std::cout << "\n";
    }
}

void setFactory(std::string const &name, FactoryType type, 
		std::string const &status)
{
    if (status == "on") {
	Console::setFactoryActive(name, type, true);
    }
    else if (status == "off") {
	Console::setFactoryActive(name, type, false);
    }
    else {
	std::cout << "status should be \"on\" or \"off\"";
    }
}
	    
