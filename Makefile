.PHONY: all
all: lib drum client proto aci

.PHONY: aci
aci:
	cd aci/; make

.PHONY: proto
proto:
	cd proto/; make

.PHONY: drum
drum:
	cd drum/; make

.PHONY: client
client:
	cd client/; make

lib:
	mkdir -p lib/
