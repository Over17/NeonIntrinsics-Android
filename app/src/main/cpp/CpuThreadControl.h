#pragma once

void SetCurrentThreadAffinity(unsigned int affinity);
unsigned int GetLittleCoreAffinity();
unsigned int GetBigCoreAffinity();
