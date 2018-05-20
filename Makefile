
ROOTCFLAGS = `root-config --cflags`
ROOTLIBS   = `root-config --libs`

CXXFLAGS += -I. -Wall -std=c++11 


%.o : %.c
	$(RM) $@
	$(CC) -c $(CFLAGS) -o $@ $<
	
%.o : %.cc
	$(RM) $@
	$(CXX) -c $(CXXFLAGS) -o $@ $*.cc


OBJS    = FormFactor.o NuFlux.o xscns.o DetectorResponse.o


LIBBASE   = DiffSpec
VER       = 1.0
TAG       = 
LIBALIAS  = $(LIBBASE)$(TAG)
LIBNAME   = $(LIBALIAS)_$(VER)

libdiffspec     = lib$(LIBNAME).a


targets = $(libdiffspec) formfactors diff_rates reactor_diff_rates nufluxes maptest detresp


$(libdiffspec) : $(OBJS)
	$(RM) $@
	ar clq $@ $(OBJS)
	ranlib $@
	

diff_rates: diff_rates.o $(libdiffspec) 
	$(RM) $@
	$(CXX) -o $@ $(CXXFLAGS) -L. $^ $(ROOTLIBS)


.PHONY: diff_rates.o
diff_rates.o: 
	$(CXX) -o diff_rates.o $(ROOTCFLAGS) $(CXXFLAGS) -c diff_rates.cc


get_time_integral: get_time_integral.o $(libdiffspec) 
	$(RM) $@
	$(CXX) -o $@ $(CXXFLAGS) -L. $^ $(ROOTLIBS)


.PHONY: get_time_integral.o
get_time_integral.o: 
	$(CXX) -o get_time_integral.o $(ROOTCFLAGS) $(CXXFLAGS) -c get_time_integral.cc



get_flavor_weight.o:
	$(CXX) -o get_flavor_weight.o $(ROOTCFLAGS) $(CXXFLAGS) -c get_flavor_weight.cc

csi_check: csi_check.o $(libdiffspec) 
	$(RM) $@
	$(CXX) -o $@ $(CXXFLAGS) get_flavor_weight.o -L. $^ $(ROOTLIBS) 


.PHONY: csi_check.o
csi_check.o: 
	$(CXX) -o csi_check.o $(ROOTCFLAGS) $(CXXFLAGS) -c csi_check.cc



reactor_diff_rates: reactor_diff_rates.o $(libdiffspec) 
	$(RM) $@
	$(CXX) -o $@ $(CXXFLAGS) -L. $^ $(ROOTLIBS)


.PHONY: reactor_diff_rates.o
reactor_diff_rates.o: 
	$(CXX) -o reactor_diff_rates.o $(ROOTCFLAGS) $(CXXFLAGS) -c reactor_diff_rates.cc


supernova_diff_rates: supernova_diff_rates.o $(libdiffspec) 
	$(RM) $@
	$(CXX) -o $@ $(CXXFLAGS) -L. $^ $(ROOTLIBS)


.PHONY: supernova_diff_rates.o
supernova_diff_rates.o: 
	$(CXX) -o supernova_diff_rates.o $(ROOTCFLAGS) $(CXXFLAGS) -c supernova_diff_rates.cc



formfactors: formfactors.o $(libdiffspec) 
	$(RM) $@
	$(CXX) -o $@ $(CXXFLAGS) -L. $^ $(ROOTLIBS)


.PHONY: formfactors.o
formfactors.o: 
	$(CXX) -o formfactors.o $(ROOTCFLAGS) $(CXXFLAGS) -c formfactors.cc


nufluxes: nufluxes.o $(libdiffspec) 
	$(RM) $@
	$(CXX) -o $@ $(CXXFLAGS) -L. $^ $(ROOTLIBS)


.PHONY: nufluxes.o
nufluxes.o: 
	$(CXX) -o nufluxes.o $(ROOTCFLAGS) $(CXXFLAGS) -c nufluxes.cc


check_fluxes: check_fluxes.o get_flavor_weight.o $(libdiffspec) 
	$(RM) $@
	$(CXX) -o $@ $(CXXFLAGS) -L. $^ $(ROOTLIBS)


.PHONY: check_fluxes.o 
check_fluxes.o: 
	$(CXX) -o check_fluxes.o $(ROOTCFLAGS) $(CXXFLAGS) -c check_fluxes.cc



detresp: detresp.o $(libdiffspec) 
	$(RM) $@
	$(CXX) -o $@ $(CXXFLAGS) -L. $^ $(ROOTLIBS)

.PHONY: detresp.o
detresp.o: 
	$(CXX) -o detresp.o $(ROOTCFLAGS) $(CXXFLAGS) -c detresp.cc




maptest: maptest.o $(libdiffspec) 
	$(RM) $@
	$(CXX) -o $@ $(CXXFLAGS) -L. $^ $(ROOTLIBS)


.PHONY: maptest.o
maptest.o: 
	$(CXX) -o maptest.o $(ROOTCFLAGS) $(CXXFLAGS) -c maptest.cc




	
.PHONY: all
all: $(targets)
	
	
.PHONY: clean
clean:
	$(RM) $(targets) *.o
	
emptyrule:: $(libdiffspec)

	
