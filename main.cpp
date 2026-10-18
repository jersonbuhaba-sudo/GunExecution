// GunExecution - GTA SA Android (AML) plugin
// Kapag nag-fire ka ng kahit anong baril sa NPC na point blank, patay agad (execution).
// UNTESTED: offsets are for 32-bit libGTASA 2.00. Adjust OFF_* if different version.

#include <mod/amlmod.h>
#include <mod/logger.h>
#include <math.h>

MYMODCFG(net.you.gunexecution, GunExecution, 1.0, You)
BEGIN_DEPLIST()
END_DEPLIST()

// ---- Settings ----
static const float EXEC_RANGE = 1.5f;   // point blank distance (meters)

// ---- Offsets (palitan kung iba ang version mo) ----
static const int OFF_POS        = 0x04;   // CEntity::m_placement.pos (kapag walang matrix)
static const int OFF_MATRIX     = 0x14;   // CEntity::m_matrix pointer
static const int OFF_MATRIX_POS = 0x30;   // CMatrix::pos
static const int OFF_TYPE       = 0x36;   // CEntity::m_nType (low 3 bits), 3 = ped
static const int OFF_HEALTH     = 0x540;  // CPed::m_fHealth

void*     hGTASA = nullptr;
uintptr_t pGTASA = 0;

typedef void* (*FindPlayerPed_t)(int);
typedef bool  (*Fire_t)(void*, void*, float*, float*, void*, float*, float*);

static FindPlayerPed_t FindPlayerPed = nullptr;
static Fire_t          Fire_orig     = nullptr;

static inline float* GetPos(void* ent)
{
    void* matrix = *(void**)((char*)ent + OFF_MATRIX);
    if (matrix) return (float*)((char*)matrix + OFF_MATRIX_POS);
    return (float*)((char*)ent + OFF_POS);
}

static inline bool IsPed(void* ent)
{
    return ent && ((*(unsigned char*)((char*)ent + OFF_TYPE)) & 7) == 3;
}

static bool Fire_hook(void* weapon, void* owner, float* origin, float* effectOrigin,
                      void* targetEntity, float* target, float* driveby)
{
    bool result = Fire_orig(weapon, owner, origin, effectOrigin, targetEntity, target, driveby);

    void* player = FindPlayerPed ? FindPlayerPed(0) : nullptr;
    if (result && player && owner == player && IsPed(targetEntity) && targetEntity != player)
    {
        float* p = GetPos(player);
        float* t = GetPos(targetEntity);
        float dx = p[0] - t[0], dy = p[1] - t[1], dz = p[2] - t[2];
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);

        if (dist <= EXEC_RANGE)
        {
            float* health = (float*)((char*)targetEntity + OFF_HEALTH);
            if (*health > 0.0f) *health = 0.0f;   // execute
        }
    }
    return result;
}

extern "C" void OnModLoad()
{
    logger->SetTag("GunExecution");

    pGTASA = aml->GetLib("libGTASA.so");
    hGTASA = aml->GetLibHandle("libGTASA.so");
    if (!pGTASA || !hGTASA) { logger->Error("libGTASA not found"); return; }

    FindPlayerPed = (FindPlayerPed_t)aml->GetSym(hGTASA, "_Z13FindPlayerPedi");
    void* fire = aml->GetSym(hGTASA, "_ZN7CWeapon4FireEP4CPedP7CVectorS3_P7CEntityS3_S3_");

    if (!FindPlayerPed || !fire) { logger->Error("symbols not found"); return; }

    aml->Hook(fire, (void*)Fire_hook, (void**)&Fire_orig);
    logger->Info("Loaded");
}
