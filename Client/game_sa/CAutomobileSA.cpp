/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        game_sa/CAutomobileSA.cpp
 *  PURPOSE:     Automobile vehicle entity
 *
 *  Multi Theft Auto is available from http://www.multitheftauto.com/
 *
 *****************************************************************************/

#include "StdInc.h"

void CAutomobileSAInterface::StaticSetHooks()
{
    HookInstall(0x6A3BD0, &CAutomobileSAInterface::DoNitroEffect);
}

void CAutomobileSAInterface::DoNitroEffect(float power)
{
    auto          modelInfo = CModelInfoSAInterface::GetModelInfo(m_nModelIndex)->AsVehicleModelInfoPtr();
    SVehicleDummy firstExhaustDummy;
    SVehicleDummy secondExhaustDummy;
    firstExhaustDummy.m_position = modelInfo->pVisualInfo->vecDummies[eVehicleModelDummy::e::EXHAUST];
    secondExhaustDummy.m_position = firstExhaustDummy.m_position;
    secondExhaustDummy.m_position.fX *= -1.0f;
    if (pGame->m_pVehicleAddExhaustParticlesHandler)
        pGame->m_pVehicleAddExhaustParticlesHandler(this, firstExhaustDummy, secondExhaustDummy);
    bool  hasDoubleExhaust = secondExhaustDummy.m_visibleSet ? secondExhaustDummy.m_visible : pHandlingData->m_bDoubleExhaust;
    bool  firstExhaustSubmergedInWater = false;
    bool  secondExhaustSubmergedInWater = false;
    float level = 0.0f;
    if (bTouchingWater)
    {
        CVector point = *Placeable.matrix * firstExhaustDummy.m_position;
        if (CWaterLevelSA::GetWaterLevel(point.fX, point.fY, point.fZ, &level, true, nullptr))
        {
            if (level >= point.fZ)
                firstExhaustSubmergedInWater = true;
        }
    }
    if (hasDoubleExhaust && !bTouchingWater)
    {
        CVector point = *Placeable.matrix * secondExhaustDummy.m_position;
        if (CWaterLevelSA::GetWaterLevel(point.fX, point.fY, point.fZ, &level, true, nullptr))
        {
            if (level >= point.fZ)
                secondExhaustSubmergedInWater = true;
        }
    }
    RwFrame*  frame = reinterpret_cast<RwFrame*>(rwObjectGetParent(m_pRwObject));
    RwMatrix* rwMatrix = RwFrameGetMatrix(frame);
    if (firstExhaustDummy.m_visible)
    {
        CFxSystemSAInterface* firstExhaustFxSystem = m_exhaustNitroFxSystem[0];
        if (firstExhaustFxSystem)
        {
            firstExhaustFxSystem->SetConstTime(1, fabs(power));
            eFxSystemPlayStatus playStatus = firstExhaustFxSystem->GetPlayStatus();
            if (playStatus == eFxSystemPlayStatus::FX_PLAYING && firstExhaustSubmergedInWater)
                firstExhaustFxSystem->Stop();
            else if (playStatus == eFxSystemPlayStatus::FX_STOPPED && !firstExhaustSubmergedInWater)
                firstExhaustFxSystem->Play();
        }
        else if (!firstExhaustSubmergedInWater && rwMatrix)
        {
            firstExhaustFxSystem = CFxManagerSA::g_fxMan.CreateFxSystem("nitro", (RwV3d*)&firstExhaustDummy.m_position, rwMatrix, true);
            m_exhaustNitroFxSystem[0] = firstExhaustFxSystem;
            if (firstExhaustFxSystem)
            {
                firstExhaustFxSystem->SetLocalParticles(true);
                firstExhaustFxSystem->Play();
            }
        }
    }
    if (hasDoubleExhaust)
    {
        CFxSystemSAInterface* secondExhaustFxSystem = m_exhaustNitroFxSystem[1];
        if (secondExhaustFxSystem)
        {
            secondExhaustFxSystem->SetConstTime(1, fabs(power));
            if (secondExhaustFxSystem->GetPlayStatus() == eFxSystemPlayStatus::FX_PLAYING && secondExhaustSubmergedInWater)
                secondExhaustFxSystem->Stop();
            else if (secondExhaustFxSystem->GetPlayStatus() == eFxSystemPlayStatus::FX_STOPPED && !secondExhaustSubmergedInWater)
                secondExhaustFxSystem->Play();
        }
        else if (!firstExhaustSubmergedInWater && rwMatrix)
        {
            secondExhaustFxSystem = CFxManagerSA::g_fxMan.CreateFxSystem("nitro", (RwV3d*)&secondExhaustDummy.m_position, rwMatrix, true);
            m_exhaustNitroFxSystem[1] = secondExhaustFxSystem;
            if (secondExhaustFxSystem)
            {
                secondExhaustFxSystem->SetLocalParticles(1);
                secondExhaustFxSystem->Play();
            }
        }
    }
}

/**
 * \todo Spawn automobiles with engine off
 */
CAutomobileSA::CAutomobileSA(eVehicleTypes dwModelID, unsigned char ucVariation, unsigned char ucVariation2) : CVehicleSA(dwModelID, ucVariation, ucVariation2)
{
    DEBUG_TRACE("CAutomobileSA::CAutomobileSA( eVehicleTypes dwModelID ):CVehicleSA( dwModelID )");

    m_automobileInterface = static_cast<CAutomobileSAInterface*>(this->GetInterface());
    for (int i = 0; i < MAX_DOORS; i++)
    {
        this->door[i].SetInterface(&m_automobileInterface->m_doors[i]);
    }
}

CAutomobileSA::CAutomobileSA(CAutomobileSAInterface* automobile) : m_automobileInterface(automobile)
{
}

CAutomobileSA::~CAutomobileSA()
{
}

bool CAutomobileSA::BurstTyre(DWORD dwTyreID)
{
    DEBUG_TRACE("bool CAutomobileSA::BurstTyre ( DWORD dwTyreID )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_BurstTyre;
    bool  bReturn;

    _asm
    {
        mov     ecx, dwThis
        push    dwTyreID
        call    dwFunc;
        mov     bReturn, al
    }

    return bReturn;
}

bool CAutomobileSA::BreakTowLink()
{
    DEBUG_TRACE("bool CAutomobileSA::BreakTowLink ( void )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_BreakTowLink;
    bool  bReturn;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        mov     bReturn, al
    }

    return bReturn;
}

void CAutomobileSA::BlowUpCar(CEntity* pEntity)
{
    DEBUG_TRACE("void CAutomobileSA::BlowUpCar ( CEntity* pEntity )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_BlowUpCar;

    _asm
    {
        mov     ecx, dwThis
        push    pEntity
        call    dwFunc
    }
}

void CAutomobileSA::BlowUpCarsInPath()
{
    DEBUG_TRACE("void CAutomobileSA::BlowUpCarsInPath ( void )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_BlowUpCarsInPath;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }
}

void CAutomobileSA::CloseAllDoors()
{
    DEBUG_TRACE("void CAutomobileSA::CloseAllDoors ( void )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_CloseAllDoors;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }
}

void CAutomobileSA::CloseBoot()
{
    DEBUG_TRACE("void CAutomobileSA::CloseBoot ( void )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_CloseBoot;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }
}

float CAutomobileSA::FindWheelWidth(bool bUnknown)
{
    DEBUG_TRACE("float CAutomobileSA::FindWheelWidth ( bool bUnknown )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwUnknown = (DWORD)bUnknown;
    DWORD dwFunc = FUNC_CAutomobile_FindWheelWidth;
    float fReturn;

    _asm
    {
        mov     ecx, dwThis
        push    dwUnknown
        call    dwFunc
        fstp    fReturn;
    }

    return fReturn;
}

/*
void CAutomobileSA::Fix ( void )
{
    DEBUG_TRACE("void CAutomobileSA::Fix ( void )");
    DWORD dwThis = (DWORD) GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_Fix;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }
}*/

void CAutomobileSA::FixDoor(int iCarNodeIndex, eDoorsSA Door)
{
    DEBUG_TRACE("void CAutomobileSA::FixDoor ( int iCarNodeIndex, eDoorsSA Door )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_FixDoor;

    _asm
    {
        mov     ecx, dwThis
        push    Door
        push    iCarNodeIndex
        call    dwFunc
    }
}

int CAutomobileSA::FixPanel(int iCarNodeIndex, ePanelsSA Panel)
{
    DEBUG_TRACE("int CAutomobileSA::FixPanel ( int iCarNodeIndex, ePanelsSA Panel )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_FixPanel;
    int   iReturn;

    _asm
    {
        mov     ecx, dwThis
        push    Panel
        push    iCarNodeIndex
        call    dwFunc
        mov     iReturn, eax
    }

    return iReturn;
}

bool CAutomobileSA::GetAllWheelsOffGround()
{
    DEBUG_TRACE("bool CAutomobileSA::GetAllWheelsOffGround ( void )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_GetAllWheelsOffGround;
    bool  bReturn;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        mov     bReturn, al
    }

    return bReturn;
}

float CAutomobileSA::GetCarPitch()
{
    DEBUG_TRACE("float CAutomobileSA::GetCarPitch ( void )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_GetCarPitch;
    float fReturn;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        fstp    fReturn
    }

    return fReturn;
}

float CAutomobileSA::GetCarRoll()
{
    DEBUG_TRACE("float CAutomobileSA::GetCarRoll ( void )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_GetCarRoll;
    float fReturn;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        fstp    fReturn
    }

    return fReturn;
}

void CAutomobileSA::GetComponentWorldPosition(int iComponentID, CVector* pVector)
{
    DEBUG_TRACE("void CAutomobileSA::GetComponentWorldPosition ( int iComponentID, CVector* pVector)");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_GetComponentWorldPosition;

    _asm
    {
        mov     ecx, dwThis
        push    pVector
        push    iComponentID
        call    dwFunc
    }
}

/*float CAutomobileSA::GetHeightAboveRoad ( void )
{

}*/

DWORD CAutomobileSA::GetNumContactWheels()
{
    DEBUG_TRACE("DWORD CAutomobileSA::GetNumContactWheels ( void )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_GetNumContactWheels;
    DWORD dwReturn;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        mov     dwReturn, eax
    }

    return dwReturn;
}

float CAutomobileSA::GetRearHeightAboveRoad()
{
    DEBUG_TRACE("float CAutomobileSA::GetRearHeightAboveRoad ( void )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_GetRearHeightAboveRoad;
    float fReturn;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        fstp    fReturn
    }

    return fReturn;
}

bool CAutomobileSA::IsComponentPresent(int iComponentID)
{
    DEBUG_TRACE("bool CAutomobileSA::IsComponentPresent ( int iComponentID )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_IsComponentPresent;
    bool  bReturn;

    _asm
    {
        mov     ecx, dwThis
        push    iComponentID
        call    dwFunc
        mov     bReturn, al
    }

    return bReturn;
}

bool CAutomobileSA::IsDoorClosed(eDoorsSA Door)
{
    DEBUG_TRACE("bool CAutomobileSA::IsDoorClosed ( eDoorsSA Door )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_IsDoorClosed;
    bool  bReturn;

    _asm
    {
        mov     ecx, dwThis
        push    Door
        call    dwFunc
        mov     bReturn, al
    }

    return bReturn;
}

bool CAutomobileSA::IsDoorFullyOpen(eDoorsSA Door)
{
    DEBUG_TRACE("bool CAutomobileSA::IsDoorFullyOpen ( eDoorsSA Door )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_IsDoorFullyOpen;
    bool  bReturn;

    _asm
    {
        mov     ecx, dwThis
        push    Door
        call    dwFunc
        mov     bReturn, al
    }

    return bReturn;
}

bool CAutomobileSA::IsDoorMissing(eDoorsSA Door)
{
    DEBUG_TRACE("bool CAutomobileSA::IsDoorMissing ( eDoorsSA Door )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_IsDoorMissing;
    bool  bReturn;

    _asm
    {
        mov     ecx, dwThis
        push    Door
        call    dwFunc
        mov     bReturn, al
    }

    return bReturn;
}

bool CAutomobileSA::IsDoorReady(eDoorsSA Door)
{
    DEBUG_TRACE("bool CAutomobileSA::IsDoorReady ( eDoorsSA Door )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_IsDoorReady;
    bool  bReturn;

    _asm
    {
        mov     ecx, dwThis
        push    Door
        call    dwFunc
        mov     bReturn, al
    }

    return bReturn;
}

bool CAutomobileSA::IsInAir()
{
    DEBUG_TRACE("bool CAutomobileSA::IsInAir ( void )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_IsInAir;
    bool  bReturn;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        mov     bReturn, al
    }

    return bReturn;
}

bool CAutomobileSA::IsOpenTopCar()
{
    DEBUG_TRACE("bool CAutomobileSA::IsOpenTopCar ( void )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_IsOpenTopCar;
    bool  bReturn;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        mov     bReturn, al
    }

    return bReturn;
}

void CAutomobileSA::PlayCarHorn()
{
    DEBUG_TRACE("void CAutomobileSA::PlayCarHorn ( void )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_PlayCarHorn;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }
}

void CAutomobileSA::PopBoot()
{
    DEBUG_TRACE("void CAutomobileSA::PopBoot ( void )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_PopBoot;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }
}

void CAutomobileSA::PopBootUsingPhysics()
{
    DEBUG_TRACE("void CAutomobileSA::PopBootUsingPhysics ( void )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_PopBootUsingPhysics;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }
}

void CAutomobileSA::PopDoor(int iCarNodeIndex, eDoorsSA Door, bool bUnknown)
{
    DEBUG_TRACE("void CAutomobileSA::PopDoor ( int iCarNodeIndex, eDoorsSA Door, bool bUnknown )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwUnknown = (DWORD)bUnknown;
    DWORD dwFunc = FUNC_CAutomobile_PopDoor;

    _asm
    {
        mov     ecx, dwThis
        push    dwUnknown
        push    Door
        push    iCarNodeIndex
        call    dwFunc
    }
}

void CAutomobileSA::PopPanel(int iCarNodeIndex, ePanelsSA Panel, bool bFallOffFast)
{
    DEBUG_TRACE("void CAutomobileSA::PopPanel ( int iCarNodeIndex, ePanelsSA Panel, bool bFallOffFast )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFallOffFast = (DWORD)bFallOffFast;
    DWORD dwFunc = FUNC_CAutomobile_PopPanel;

    _asm
    {
        mov     ecx, dwThis
        push    dwFallOffFast
        push    Panel
        push    iCarNodeIndex
        call    dwFunc
    }
}

void CAutomobileSA::ResetSuspension()
{
    DEBUG_TRACE("void CAutomobileSA::ResetSuspension ( void )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile_ResetSuspension;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }
}

void CAutomobileSA::SetRandomDamage(bool bUnknown)
{
    DEBUG_TRACE("void CAutomobileSA::SetRandomDamage ( bool bUnknown )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwUnknown = (DWORD)bUnknown;
    DWORD dwFunc = FUNC_CAutomobile_SetRandomDamage;

    _asm
    {
        mov     ecx, dwThis
        push    dwUnknown
        call    dwFunc
    }
}

void CAutomobileSA::SetTaxiLight(bool bState)
{
    DEBUG_TRACE("void CAutomobileSA::SetTaxiLight ( bool bState )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwState = (DWORD)bState;
    DWORD dwFunc = FUNC_CAutomobile_SetTaxiLight;

    _asm
    {
        mov     ecx, dwThis
        push    dwState
        call    dwFunc
    }
}

void CAutomobileSA::SetTotalDamage(bool bUnknown)
{
    DEBUG_TRACE("void CAutomobileSA::SetTotalDamage (bool bUnknown )");
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwUnknown = (DWORD)bUnknown;
    DWORD dwFunc = FUNC_CAutomobile_SetTotalDamage;

    _asm
    {
        mov     ecx, dwThis
        push    dwUnknown
        call    dwFunc
    }
}

CPhysical* CAutomobileSA::SpawnFlyingComponent(int iCarNodeIndex, int iUnknown)
{
    DEBUG_TRACE("CPhysical* CAutomobileSA::SpawnFlyingComponent ( int iCarNodeIndex, int iUnknown )");
    DWORD      dwThis = (DWORD)GetInterface();
    DWORD      dwFunc = FUNC_CAutomobile_SpawnFlyingComponent;
    CPhysical* pReturn;

    _asm
    {
        mov     ecx, dwThis
        push    iUnknown
        push    iCarNodeIndex
        call    dwFunc
        mov     pReturn, eax
    }

    return pReturn;
}

CDoor* CAutomobileSA::GetDoor(eDoors doorID)
{
    DEBUG_TRACE("CDoor * CAutomobileSA::GetDoor(eDoors doorID)");
    return &this->door[doorID];
}

void CAutomobileSA::SetNitroFxSystemPosition(int id, const CVector& position)
{
    auto nitroFxSystem = m_automobileInterface->m_exhaustNitroFxSystem[id];
    if (nitroFxSystem)
        nitroFxSystem->SetOffsetPos((RwV3d*)&position);
}

void CAutomobileSA::SetNitroFxSystemVisible(int id, bool visible)
{
    // CAutomobileSAInterface::DoNitroEffect will recreate the fx system
    // if it's not there when vehicle->IsDummyVisible returns true, and
    // that's why we don't need to create the fx system here when
    // "visible" parameter is true.
    if (!visible)
    {
        auto nitroFxSystem = m_automobileInterface->m_exhaustNitroFxSystem[id];
        if (nitroFxSystem)
        {
            nitroFxSystem->Kill();
            CFxManagerSA::g_fxMan.DestroyFxSystem(nitroFxSystem);
            m_automobileInterface->m_exhaustNitroFxSystem[id] = nullptr;
        }
    }
}
