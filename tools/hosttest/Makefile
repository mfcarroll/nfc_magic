# Host-side tests for the ISO15693 sweep logic. Dev-only: application.fam excludes tools/ from the FAP
# build, so nothing here ships or affects the firmware build.
#
#   make          build and run
#   make clean

CC ?= cc
CFLAGS ?= -std=gnu11 -g -O0 -Wall -Wextra -Wno-unused-parameter
INCLUDES := -Ifakes -I.
BUILD := build

TESTS := test_wipe_sweep test_clone_blocks

.PHONY: all run clean
all: run

$(BUILD):
	@mkdir -p $(BUILD)

# -MMD/-MP generate the real prerequisite list, which MUST include the code under test: the tests
# #include iso15693_poller.c, and without a depfile an edit to the poller leaves a stale binary passing.
# That happened once -- a fixed bug still reported as failing, because make had nothing to rebuild on.
DEPFLAGS = -MMD -MP -MF $(BUILD)/$*.d

$(BUILD)/%: %.c fake_tag.c fake_tag.h | $(BUILD)
	$(CC) $(CFLAGS) $(DEPFLAGS) $(INCLUDES) -o $@ $< fake_tag.c

run: $(addprefix $(BUILD)/,$(TESTS))
	@fail=0; for t in $^; do echo; ./$$t || fail=1; done; exit $$fail

clean:
	rm -rf $(BUILD)

-include $(wildcard $(BUILD)/*.d)
