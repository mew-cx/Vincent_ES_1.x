/*
//
//Copyright (C) 2005-2006  Falanx Microsystems AS
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of Falanx Microsystems AS nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//POSSIBILITY OF SUCH DAMAGE.
//
*/
#include "ARBOptimization.h"
#include "Util.h"

#include <string>
#include <map>
#include <queue>

using namespace std;

// Whether an instruction is a texture instruction
bool isTextureOp(ARBInstructionType type) {
	switch (type) {
		case EInstTEX:
		case EInstTXB:
		case EInstTXP:
			return true;
		default:
			return false;
	}
}

// Propagate samplers to where they are used
void ARBPropagateTextures(ARBInstructionList& instructions) {
	ARBInstructionList newList;
	map<string, ARBVar> texmaps;
	for (unsigned int i = 0; i < instructions.size(); ++i) {
		ARBVar output = instructions[i].getOutputVar();
		vector<ARBVar> input = instructions[i].getInputVarList();
		if (instructions[i].getInstructionType() == EInstMOV && input.size() == 1 && input[0].isSampler()) {
			texmaps.insert(pair<string, ARBVar>(output.getasm(), input[0]));
		} else if (isTextureOp(instructions[i].getInstructionType()) && input.size() >= 2) {
			map<string, ARBVar>::const_iterator tm = texmaps.find(input[1].getasm());
			if (tm != texmaps.end()) {
				input[1] = tm->second;
			}
			newList.push_back(ARBInstruction(instructions[i].getInstructionType(), output, input));
		} else {
			newList.push_back(ARBInstruction(instructions[i].getInstructionType(), output, input));
		}
	}
	for (map<string, ARBVar>::const_iterator i = texmaps.begin(); i != texmaps.end(); ++i) {
		msg() << i->first << " -> " << i->second.getasm() << "\n";
	}
	instructions = newList;
}

// Propagate cases where a variable is assigned once and then only copied to another variable
void ARBCopyPropagation(ARBInstructionList& instructions) {
	// Record variable usage
	set<string> cannotBePropagated;
	map<string, int> usedAsResult;
	map<string, int> usedAsInput;
	map<string, ARBVar> usedFor;
	for (unsigned int i = 0; i < instructions.size(); ++i) {
		ARBVar output = instructions[i].getOutputVar();
		const vector<ARBVar>& input = instructions[i].getInputVarList();
		usedAsResult[output.getasm()]++;
		if (output.isSwizzled() || !output.isTemporary()) {
			cannotBePropagated.insert(output.getasm());
		}
		for (unsigned int j = 0; j < input.size(); ++j) {
			if (input[j].isTemporary() && !usedAsResult[input[j].getasm()]) {
				failmsg() << "Warning: " << input[j].getasm() << " used before assignment\n";
				cannotBePropagated.insert(input[j].getasm());
			}
			usedAsInput[input[j].getasm()]++;
			if (instructions[i].getInstructionType() != EInstMOV) {
				cannotBePropagated.insert(input[j].getasm());
			} else {
				usedFor.insert(pair<string, ARBVar>(input[j].getasm(), output));
			}
			if (input[j].isSwizzled() || !input[j].isTemporary()) {
				cannotBePropagated.insert(input[j].getasm());
			}
		}
	}

	// Find variables that can be propagated
	set<string> propagationVars;
	for (map<string, int>::const_iterator i = usedAsResult.begin(); i != usedAsResult.end(); ++i) {
		if (cannotBePropagated.count(i->first)) {
			continue;
		}
		if (i->second == 1 && usedAsInput[i->first] == 1) {
			propagationVars.insert(i->first);
			msg() << "propagating " << i->first << "\n";
		}
	}

	// Propagate variables
	if (propagationVars.size() == 0) {
		return;
	}
	ARBInstructionList newList;
	for (unsigned int i = 0; i < instructions.size(); ++i) {
		ARBVar output = instructions[i].getOutputVar();
		const vector<ARBVar>& input = instructions[i].getInputVarList();
		if (instructions[i].getInstructionType() == EInstMOV && input.size() == 1 && propagationVars.count(input[0].getasm())) {
			// MOV symbol_gl_FragColor, tempvar_16
			// Discard instruction
		} else if (propagationVars.count(output.getasm())) {
			// ADD tempvar_16, symbol_tmp, const_vec4_0_0_0x5_1
			// Replace output variable
			map<string, ARBVar>::const_iterator uf = usedFor.find(output.getasm());
			if (uf != usedFor.end()) {
				ARBVar target = uf->second;
				msg() << "changing instr to use " << target.tostr() << " because that's what " << output.getasm() << " is used for\n";
				while (propagationVars.count(target.getasm())) {
					uf = usedFor.find(target.getasm());
					if (uf == usedFor.end()) {
						break;
					} else {
						target = uf->second;
						msg() << "  however, " << target.tostr() << " should be used instead\n";
					}
				}
				ARBInstruction inst = ARBInstruction(instructions[i].getInstructionType(), target, input);
				newList.push_back(inst);
			} else {
				newList.push_back(instructions[i]);
			}
		} else {
			newList.push_back(instructions[i]);
		}
	}
	instructions = newList;
}

// Coalesce temporaries that don't overlap in time
void ARBCoalesceTemporaries(ARBInstructionList& instructions) {
	// Record last time of usage
	map<string, int> firstUsedMap;
	map<string, int> lastUsedMap;
	set<string> ignoredVars;
	for (unsigned int i = 0; i < instructions.size(); ++i) {
		ARBVar output = instructions[i].getOutputVar();
		const vector<ARBVar>& input = instructions[i].getInputVarList();
		if (output.isTemporary()) {
			if (output.isSwizzled()) {
				ignoredVars.insert(output.getasm());
			}
			firstUsedMap.insert(pair<string, int>(output.getasm(), i));
			lastUsedMap[output.getasm()] = i;
		}
		for (unsigned int j = 0; j < input.size(); ++j) {
			if (input[j].isTemporary()) {
				if (input[j].isSwizzled()) {
					ignoredVars.insert(input[j].getasm());
				}
				firstUsedMap.insert(pair<string, int>(input[j].getasm(), i));
				lastUsedMap[input[j].getasm()] = i;
			}
		}
	}

	// Find temporaries that can use previous variables that aren't used anymore instead
	priority_queue<pair<int, string> > firstUsed;
	for (map<string, int>::const_iterator i = firstUsedMap.begin(); i != firstUsedMap.end(); ++i) {
		if (!ignoredVars.count(i->first)) {
			firstUsed.push(pair<int, string>(i->second, i->first));
		}
	}
	priority_queue<pair<int, string> > lastUsed;
	for (map<string, int>::const_iterator i = lastUsedMap.begin(); i != lastUsedMap.end(); ++i) {
		if (!ignoredVars.count(i->first)) {
			lastUsed.push(pair<int, string>(i->second, i->first));
		}
	}

// 	msg() << "First usage:\n";
// 	priority_queue<pair<int, string> > firstUsed2 = firstUsed;
// 	while (firstUsed2.size()) {
// 		const pair<int, string>& value = firstUsed2.top();
// 		msg() << value.first << " - " << value.second << "\n";
// 		firstUsed2.pop();
// 	}
//
// 	msg() << "Last usage:\n";
// 	priority_queue<pair<int, string> > lastUsed2 = lastUsed;
// 	while (lastUsed2.size()) {
// 		const pair<int, string>& value = lastUsed2.top();
// 		msg() << value.first << " - " << value.second << "\n";
// 		lastUsed2.pop();
// 	}

	map<string, string> remapping;
	set<string> remappedTo;
	while (firstUsed.size()) {
		const pair<int, string>& first = firstUsed.top();
		while (lastUsed.size()) {
			const pair<int, string>& last = lastUsed.top();
// 			msg() << "Checking " << first.second << " (" << first.first << ") vs " << last.second << " (" << last.first << ")\n";
			if (first.first > last.first) {
				msg() << "Making " << first.second << " use " << last.second << " instead\n";
				remapping[first.second] = last.second;
				remappedTo.insert(last.second);
				firstUsed.pop();
				const pair<int, string>& first = firstUsed.top();
			}
			lastUsed.pop();
		}
		firstUsed.pop();
	}
	map<string, string> reducedMap;
	for (map<string, string>::const_iterator i = remapping.begin(); i != remapping.end(); ++i) {
		string target = i->second;
		while (remapping.find(target) != remapping.end()) {
			target = remapping[target];
		}
		if (target != i->second) {
			msg() << "  also making " << i->first << " use " << target << " instead of " + i->second + "\n";
		}
		reducedMap[i->first] = target;
	}
	remapping = reducedMap;

	// Replace temporaries in instructions
	ARBInstructionList newList;
	map<string, ARBVar> mappedVars;
	for (unsigned int i = 0; i < instructions.size(); ++i) {
		ARBInstruction inst = instructions[i];
		ARBVar output = inst.getOutputVar();
		if (remappedTo.count(output.getasm())) {
			mappedVars.insert(pair<string, ARBVar>(output.getasm(), output));
		}
		vector<ARBVar> input = inst.getInputVarList();
		for (unsigned int j = 0; j < input.size(); ++j) {
			if (remappedTo.count(input[j].getasm())) {
				mappedVars.insert(pair<string, ARBVar>(input[j].getasm(), output));
			}
		}
		if (remapping.find(output.getasm()) != remapping.end()) {
			map<string, ARBVar>::const_iterator mv = mappedVars.find(remapping[output.getasm()]);
			if (mv != mappedVars.end()) {
				output = mv->second;
			} else {
				msg() << "Warning: " << output.getasm() << " -> " << remapping[output.getasm()] << " mapping failed\n";
			}
		}
		for (unsigned int j = 0; j < input.size(); ++j) {
			if (remapping.find(input[j].getasm()) != remapping.end()) {
				map<string, ARBVar>::const_iterator mv = mappedVars.find(remapping[input[j].getasm()]);
				if (mv != mappedVars.end()) {
					input[j] = mv->second;
				} else {
					msg() << "Warning: " << output.getasm() << " -> " << remapping[input[j].getasm()] << " mapping failed\n";
				}
			}
		}
		newList.push_back(ARBInstruction(inst.getInstructionType(), output, input));
	}
	instructions = newList;
}

// Perform code optimization on the instruction list
void ARBCodeOptimization(ARBInstructionList& instructions) {
	ARBPropagateTextures(instructions);
	ARBCopyPropagation(instructions);
	ARBCoalesceTemporaries(instructions);
}
