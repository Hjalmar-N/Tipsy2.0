#pragma once

#include <cstddef>
#include <cstdint>

#include "app/models/MachineState.h"
#include "app/models/MachineStatus.h"
#include "app/models/MachinePourPlan.h"
#include "app/services/IngredientService.h"
#include "app/services/RecipeService.h"
#include "app/services/SettingsService.h"
#include "pumps/PumpController.h"

namespace tipsy::app {

enum class MachineAction : std::uint8_t {
  None = 0,
  DrinkPour,
  ManualPour,
  PrimePumps,
  FlushCleaning,
};

// Central business controller. The UI should call these methods instead of touching services directly.
class MachineController {
 public:
  MachineController(RecipeService& recipeService, IngredientService& ingredientService,
                    SettingsService& settingsService, tipsy::pumps::PumpController& pumpController);

  void begin();
  void update();

  MachineStatus selectDrink(const String& drinkId);
  MachineStatus startSelectedDrink(std::uint16_t alcoholOverrideMl = 0,
                                   std::uint8_t speedPercent = 100);
  MachineStatus startDrink(const String& drinkId, std::uint16_t alcoholOverrideMl = 0,
                           std::uint8_t speedPercent = 100);
  MachineStatus enterManualPourMode();
  MachineStatus startManualPour(const String& ingredientId, float volumeMl,
                                std::uint8_t speedPercent);
  MachineStatus enterAdminSettings();
  MachineStatus updatePumpAssignment(std::uint8_t pumpIndex, const String& ingredientId,
                                     const String& ingredientDisplayName, bool enabled);
  MachineStatus startPrimePumps();
  MachineStatus startFlushCleaning();
  MachineStatus saveSettings();
  MachineStatus enterMaintenanceMode();
  MachineStatus exitMaintenanceMode();
  MachineStatus acknowledgeComplete();
  MachineStatus stopAll();

  bool isDrinkAvailable(const String& drinkId) const;
  MachinePourPlan buildPourPlan(const String& drinkId, std::uint16_t alcoholOverrideMl = 0,
                                std::uint8_t speedPercent = 100) const;
  MachineState getCurrentState() const;
  const MachineStatus& getLastStatus() const;
  const String& getSelectedDrinkId() const;

 private:
  MachineStatus setState(MachineState state, const MachineStatus& status);
  MachineStatus fail(MachineStatusCode code, const String& message);
  MachineStatus busyStatus() const;
  bool canStartPouring() const;
  bool isIngredientAvailable(const String& ingredientId, MachineStatus& status) const;
  bool isIngredientConfiguredForDrink(const String& ingredientId, MachineStatus& status) const;
  bool recipeHasAlcoholOverrideTarget(const tipsy::domain::DrinkRecipe& recipe) const;
  bool ingredientUsesAlcoholOverride(const tipsy::domain::DrinkRecipe& recipe,
                                     std::size_t ingredientIndex) const;
  float resolveIngredientAmountMl(const tipsy::domain::DrinkRecipe& recipe,
                                  std::size_t ingredientIndex,
                                  std::uint16_t alcoholOverrideMl) const;
  bool resolvePumpForIngredient(const String& ingredientId, std::uint8_t& pumpIndex,
                                MachineStatus& status) const;

  RecipeService& recipeService_;
  IngredientService& ingredientService_;
  SettingsService& settingsService_;
  tipsy::pumps::PumpController& pumpController_;
  MachineState state_ = MachineState::Idle;
  MachineAction activeAction_ = MachineAction::None;
  MachineStatus lastStatus_ = MachineStatus::ok(MachineState::Idle, "Machine ready.");
  String selectedDrinkId_;
};

}  // namespace tipsy::app
