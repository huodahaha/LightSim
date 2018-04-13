//
// Created by Ma Huri on 4/13/18.
//

#include "trace_loader.h"

TraceFormat::TraceFormat() : pc(0), is_branch(0), branch_taken(0) {
    for (int i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
        destination_registers[i] = 0;
        destination_memory[i] = 0;
    }
    for (int i = 0; i < NUM_INSTR_SOURCES; i++) {
        source_registers[i] = 0;
        source_memory[i] = 0;
    }
}

TraceLoader::TraceLoader(string filename): reach_end(true) {
    char gzip_command[512];
    sprintf(gzip_command, "gunzip -c %s", filename);
    trace_file = popen(gzip_command, "r");
    if (trace_file == NULL) {
        printf("Unable to read the trace file %s, exiting.", filename);
        exit(-1);
    }
    reach_end = false;
}

TraceLoader::~TraceLoader() { pclose(trace_file); }

pair<bool, TraceFormat> TraceLoader::next_instruction() {
    TraceFormat ret;
    assert(!is_end());
    if( !fread(&ret, sizeof(TraceFormat), 1, trace_file)) {
        // reach the end of the file
        reach_end = true;
        return make_pair(false, ret);
    } else {
        return make_pair(true, ret);
    }
}

bool TraceLoader::is_end() const { return reach_end; }
