#ifndef OBJECT_H
#define OBJECT_H

#include "mipscode.h"

MipsNode translate_ic(InterCodes* ics);
MipsNode PrepareMipsEnv(MipsNode mc);
MipsNode trans_code(InterCode* ic, MipsNode mcnode);

void tansForLABEL(InterCode* ic, MipsNode mc);
void tansForFUNCTION(InterCode* ic, MipsNode mc);
void tansForASSIGN(InterCode* ic, MipsNode mc);
void tansForPLUS(InterCode* ic, MipsNode mc);
void tansForMINUS(InterCode* ic, MipsNode mc);
void tansForPROD(InterCode* ic, MipsNode mc);
void tansForDIV(InterCode* ic, MipsNode mc);
void tansForC2(InterCode* ic, MipsNode mc);
void tansForADDROF(InterCode* ic, MipsNode mc);
void tansForASSIGNFROMADDR(InterCode* ic, MipsNode mc);
void tansForASSIGNTOADDR(InterCode* ic, MipsNode mc);
void tansForGOTO(InterCode* ic, MipsNode mc);
void tansForIFGOTO(InterCode* ic, MipsNode mc);
void tansForRETURN(InterCode* ic, MipsNode mc);
void tansForDEC(InterCode* ic, MipsNode mc);
void tansForARG(InterCode* ic, MipsNode mc);
void tansForASSIGNCALL(InterCode* ic, MipsNode mc);
void tansForPARAM(InterCode* ic, MipsNode mc);
void tansForREAD(InterCode* ic, MipsNode mc);
void tansForWRITE(InterCode* ic, MipsNode mc);

// output
void fprint_MipsCodes(FILE* f, MipsNode mchead);

#endif
