# Quick diagnostic Makefile
RACK_DIR ?= ../..

include $(RACK_DIR)/arch.mk

test:
	@echo "ARCH_WIN = '$(ARCH_WIN)'"
	@echo "ARCH = '$(ARCH)'"
	@echo "Current platform detected"
