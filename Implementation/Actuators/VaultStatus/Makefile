CONTIKI_PROJECT = vaultstatus
all: $(CONTIKI_PROJECT)

# Include CoAP resources
MODULES_REL += ./resources
MODULES_REL += ../../Utility/JSON_SenML

CONTIKI=../../../../contiki-ng

# Include the CoAP implementation
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap

ifeq ($(TARGET), cooja)
CFLAGS += -DCOOJA
endif
CFLAGS += -DCOAP_OBSERVE_CLIENT=1

include $(CONTIKI)/Makefile.include