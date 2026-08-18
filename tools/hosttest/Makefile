# Host-side tests for the ISO15693 poller and its result screens. Dev-only: application.fam excludes
# tools/ from the FAP build, so nothing here ships or affects the firmware build.
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
# touch the radio at all, so they link a different fake.
POLLER_TESTS := test_wipe_sweep test_clone_blocks test_outcome test_write_step test_write_identity
SCENE_TESTS  := test_write_fail_scene
TESTS := $(POLLER_TESTS) $(SCENE_TESTS)

.PHONY: all run clean
all: run

$(BUILD):
	@mkdir -p $(BUILD)

# EVERY translation unit is compiled separately, each with its OWN depfile. That is not a style choice.
#
# The tests #include the shipped .c files, so the only thing that can tell make an edit to
# iso15693_poller.c or to a scene must trigger a rebuild is the depfile for the TEST unit. Compiling the
# test and its fake in one cc invocation shares a single -MF, so the fake's dependency list overwrites
# the test's -- leaving a depfile that names only the fake and its headers, and a build that never
# notices the code under test changing.
#
# That is exactly what happened here: the depfile looked present and correct for weeks while listing
# neither the test nor the poller, so `make` after a poller-only edit re-ran a stale binary and reported
# a pass that meant nothing. Verify with:
#
#     grep -c iso15693_poller.c build/test_write_identity.d      # must be non-zero
#     grep -c scenes/ build/test_write_fail_scene.d              # must be non-zero
#
$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CFLAGS) -MMD -MP -MF $(BUILD)/$*.d $(INCLUDES) -c -o $@ $<

$(addprefix $(BUILD)/,$(POLLER_TESTS)): $(BUILD)/%: $(BUILD)/%.o $(BUILD)/fake_tag.o
	$(CC) $(CFLAGS) -o $@ $^

$(addprefix $(BUILD)/,$(SCENE_TESTS)): $(BUILD)/%: $(BUILD)/%.o $(BUILD)/fake_scene.o
	$(CC) $(CFLAGS) -o $@ $^

run: $(addprefix $(BUILD)/,$(TESTS))
	@fail=0; for t in $^; do echo; ./$$t || fail=1; done; exit $$fail

clean:
	rm -rf $(BUILD)

-include $(wildcard $(BUILD)/*.d)
