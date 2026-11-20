#include "lab2/rentals/CheckoutRecord.h"
#include "lab2/bookings/HoldPolicy.h"
#include "lab2/bookings/Reservation.h"
#include "lab2/errors/AssetUnavailable.h"
#include "lab2/errors/InvalidInspection.h"
#include "lab2/errors/InvalidReservationState.h"
#include "lab2/inventory/Asset.h"

namespace lab2 {

void CheckoutRecord::verify(const Reservation& reservation, const Asset& asset,
                            const HoldPolicy& policy) const {
    reservation.requireHeld();
    const auto& request = reservation.request();
    if (reservationId != reservation.id() || asset.id() != request.assetId ||
        !clerk.mayHandle(asset.branch()) || day < request.requestedOn ||
        !policy.permitsPickup(request.period, day)) {
        throw InvalidReservationState("Проверь дату, пункт и номер выдачи.");
    }
    if (!asset.canIssue()) {
        throw AssetUnavailable("Предмет сейчас нельзя выдать.");
    }
    if (!asset.model().category.permitsIssue(briefingGiven)) {
        throw InvalidInspection("Перед выдачей проведи инструктаж.");
    }
    inspection.verifyBeforeIssue(asset.id(), day);
    kit.verify(asset.id());
}

} // namespace lab2
