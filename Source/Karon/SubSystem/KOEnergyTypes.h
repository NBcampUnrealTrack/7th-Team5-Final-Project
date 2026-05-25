// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

class IKOEnergyProducer
{
public:
    virtual ~IKOEnergyProducer() = default;

    /** 공급 가능 에너지 양 */
    virtual float GetPowerOutput(float DeltaSeconds) const = 0;
    /** 실제로 공급한 에너지 양 */
    virtual void OnPowerAccepted(float Amount) = 0;
};

class IKOEnergyConsumer
{
public:
    virtual ~IKOEnergyConsumer() = default;

    /** 소비를 원하는 양 */
    virtual float GetPowerDemand(float DeltaSeconds) const = 0;

    /** 실제로 공급받은 에너지 양 */
    virtual void OnPowerSupplied(float SuppliedAmount, float RequestedAmount) = 0;
};
