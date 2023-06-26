cmake_minimum_required(VERSION 3.24)
project(untitled2 C)

set(CMAKE_C_STANDARD 90)

add_executable(insee_n_dim_sim arbitrate.c batch.c cam.c circ_pk.c circulant.c data_generation.c dragonfly.c dtt.c event.c exd.c fattree.c get_conf.c graph.c icube.c init_functions.c ksp_routing.c list.c literal.c main.c mapping.c midimew.c misc.c pattern.c perform_mov.c pkt_mem.c print_results.c queue.c queue_inj.c request_ports.c router.c scheduling.c spanning_tree.c spinnaker.c stats.c torus.c trace.c mpa.c)
