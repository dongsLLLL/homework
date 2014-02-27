#!/bin/bash

RFILE=./rfile
SCHED=../src/sched
INS="0 1 2 3 4 5"


for f in ${INS}; do
	for s in F L S R2 R5 R20; do 
		echo "${SCHED} -s${s} input${f} ${RFILE}"
		${SCHED} -s${s} input${f} ${RFILE} > output${f}_${s}
	done
done

for f in ${INS}; do
	for s in F L S R2 R5 R20; do 
		echo "${SCHED} -v -s${s} input${f} ${RFILE}"
		${SCHED} -v -s${s} input${f} ${RFILE} > output${f}_${s}_t
	done
done
	 
