noinst_LTLIBRARIES = libglmsampler.la

libglmsampler_la_CPPFLAGS = -I$(top_srcdir)/src/include \
		-I$(top_srcdir)/src/modules/glm/CSparse \
		-I$(top_srcdir)/src/modules/glm/SSparse/config \
		-I$(top_srcdir)/src/modules/glm/SSparse/CHOLMOD/Include

libglmsampler_la_SOURCES = GLMFactory.cc GLMSampler.cc GLMMethod.cc	\
 KS.cc			\
 IWLSFactory.cc	\
 IWLS.cc LGMix.cc AuxMixPoisson.cc AuxMixBinomial.cc Outcome.cc		\
 NormalLinear.cc BinaryProbit.cc BinaryLogit.cc Classify.cc		\
 IWLSOutcome.cc HolmesHeld.cc HolmesHeldFactory.cc GLMBlock.cc		\
 GLMGibbs.cc GLMGenericFactory.cc HolmesHeldGibbs.cc PolyaGamma.cc PGcommon.cc

noinst_HEADERS = GLMFactory.h GLMSampler.h GLMMethod.h			\
  KS.h 	\
  IWLSFactory.h IWLS.h LGMix.h		\
  AuxMixPoisson.h AuxMixBinomial.h Outcome.h	\
  NormalLinear.h BinaryProbit.h BinaryLogit.h Classify.h		\
  IWLSOutcome.h HolmesHeld.h HolmesHeldFactory.h GLMBlock.h		\
  GLMGibbs.h GLMGenericFactory.h HolmesHeldGibbs.h PolyaGamma.h PG.h

check_LTLIBRARIES = libglmsamptest.la
libglmsamptest_la_SOURCES = testglmsamp.cc testglmsamp.h
libglmsamptest_la_CPPFLAGS = -I$(top_srcdir)/src/include
libglmsamptest_la_CXXFLAGS = $(CPPUNIT_CFLAGS)
