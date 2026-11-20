#include "lab2/inventory/Inspection.h"
#include "lab2/common.h"
#include "lab2/errors/InvalidInspection.h"
#include "lab2/inventory/Condition.h"

namespace lab2 {

Condition Inspection::condition() const {
    return checklist.passed() ? Condition::good : Condition::damaged;
}

void Inspection::verifyFor(Id asset, Day expectedDay) const {
    if (asset != assetId || expectedDay != day || (!checklist.passed() && note.empty())) {
        throw InvalidInspection("Проверь номер, дату осмотра и описание дефектов.");
    }
}

void Inspection::verifyBeforeIssue(Id asset, Day expectedDay) const {
    verifyFor(asset, expectedDay);
    if (condition() == Condition::damaged) {
        throw InvalidInspection("Выдача требует осмотра без дефектов.");
    }
}

} // namespace lab2
