CONTIKI_PROJECT = hvac
all: $(CONTIKI_PROJECT)

# Include CoAP resources
MODULES_REL += ./resources
MODULES_REL += ../../Utility/JSON_SenML
MODULES_REL += ../../../MachineLearning

MODULES_REL += /home/iot_ubuntu_intel/.local/lib/python3.10/site-packages/emlearn
TARGET_LIBFILES += -lm
INC += /home/iot_ubuntu_intel/.local/lib/python3.10/site-packages/emlearn

CONTIKI=../../../../contiki-ng

# Include the CoAP implementation
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap


ifeq ($(TARGET), cooja)
CFLAGS += -DCOOJA
endif
CFLAGS += -DCOAP_OBSERVE_CLIENT=1

include $(CONTIKI)/Makefile.include
