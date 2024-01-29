#pragma once

#include <L2.h>
#include <interference_analyzer.h>
#include <liveness_analyzer.h>
#include <spiller.h>

namespace L2 {
void colorGraph(const InterferenceResult &interferenceResult, const LivenessResult &livenessResult);
}