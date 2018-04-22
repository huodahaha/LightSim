#include "cmdline/cmdline.h"

#include "memory_hierarchy.h"
#include "trace_loader.h"
#include "cfg_loader.h"

#include <iostream>

void run_simulation(string cfg, unsigned int processes) {
  auto loader = CfgLoaderObj::get_instance();
  auto builder = PipeLineBuilderObj::get_instance();
  loader->parse(cfg);
  builder->load(loader->get_nodes());
  auto connectors = builder->get_connectors();
  if (connectors.size() < processes) {
    SIMLOG(SIM_ERROR, "not enough cpus in configuration\n");
    exit(1);
  }
  
  for (unsigned int i = 0; i < processes; i++) {
    connectors[i]->start();
  }

  EventEngine *evnet_queue = EventEngineObj::get_instance();
  while (true) {
    auto ret = evnet_queue->loop();
    if (ret == 0)
      break;
  }
  // print stats
  auto stats_manager = MemoryStatsManagerObj::get_instance();
  stats_manager->display_all(stdout);
}

int main(int argc, char *argv[])
{
  cmdline::parser a;
  a.add<string>("cfg", 'c', "configuration file in json format", true, "");
  a.add<unsigned int>("process", 'p', "processes to simulate", true, 0, cmdline::range(1, 8));
  a.add("verbose", 'v', "verbose output");

  a.parse_check(argc, argv);

  run_simulation(a.get<string>("cfg"), a.get<unsigned int>("process"));
  return 0;
}
