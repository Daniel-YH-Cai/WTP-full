all: opt

base:
	$(MAKE) -C WTP-base

opt:
	$(MAKE) -C WTP-opt

clean:
	$(MAKE) -C WTP-base clean
	$(MAKE) -C WTP-opt clean