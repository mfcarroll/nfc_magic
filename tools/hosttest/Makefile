# Host-side tests for the ISO15693 sweep logic. Dev-only: application.fam excludes tools/ from the FAP
# build, so nothing here ships or affects the firmware build.
#
#   make          build and run
#   make clean

CC ?= cc
CFLAGS ?= -std=gnu11 -g -O0 -Wall -Wextra -Wno-unused-parameter
# -I../.. is the app root, so the scene tests can resolve the app header's <views/...> includes.
# fakes/ stays first, so a fake always wins over anything real that happens to be reachable.
INCLUDES := -Ifakes -I. -I../..
BUILD := build

# Two groups. The poller tests drive a fake TAG; the scene tests drive fake GUI recorders and never
# touch the radio at all, so they link a different fake and get their own rule below.
POLLER_TESTS := test_wipe_sweep test_clone_blocks test_outcome test_write_step
SCENE_TESTS  := test_write_fail_scene
TESTS := $(POLLER_TESTS) $(SCENE_TESTS)

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

# More specific than the pattern rule above, so make prefers it. Same depfile reasoning: these #include
# the shipped scene .c files, and a stale binary would report a fixed screen as still broken.
$(addprefix $(BUILD)/,$(SCENE_TESTS)): $(BUILD)/%: %.c fake_scene.c fake_scene.h | $(BUILD)
	$(CC) $(CFLAGS) $(DEPFLAGS) $(INCLUDES) -o $@ $< fake_scene.c

run: $(addprefix $(BUILD)/,$(TESTS))
	@fail=0; for t in $^; do echo; ./$$t || fail=1; done; exit $$fail

clean:
	rm -rf $(BUILD)

-include $(wildcard $(BUILD)/*.d)
