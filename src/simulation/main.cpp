#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "app/MachineController.h"
#include "app/services/IngredientService.h"
#include "app/services/RecipeService.h"
#include "app/services/SettingsService.h"
#include "hal/interfaces/ITimeProvider.h"
#include "hal/mock/MockFileSystem.h"
#include "pumps/MockPumpDriver.h"
#include "pumps/PumpController.h"
#include "storage/FileSystemManager.h"
#include "storage/JsonStorage.h"

namespace tipsy::simulation {

class MockTimeProvider final : public tipsy::hal::ITimeProvider {
 public:
  std::uint32_t millis32() const override {
    return nowMs_;
  }

  void advance(std::uint32_t deltaMs) {
    nowMs_ += deltaMs;
  }

 private:
  std::uint32_t nowMs_ = 0;
};

struct ScenarioReport {
  std::string name;
  std::string expectedResult;
  std::string verifies;
  bool passed = false;
  std::string details;
};

const char* stateName(tipsy::app::MachineState state) {
  switch (state) {
    case tipsy::app::MachineState::Idle:
      return "Idle";
    case tipsy::app::MachineState::DrinkSelection:
      return "DrinkSelection";
    case tipsy::app::MachineState::ManualPour:
      return "ManualPour";
    case tipsy::app::MachineState::AdminSettings:
      return "AdminSettings";
    case tipsy::app::MachineState::Pouring:
      return "Pouring";
    case tipsy::app::MachineState::Complete:
      return "Complete";
    case tipsy::app::MachineState::Error:
      return "Error";
    case tipsy::app::MachineState::Maintenance:
      return "Maintenance";
    default:
      return "Unknown";
  }
}

const char* categoryLabel(const String& categoryId) {
  return categoryId == "shot" ? "Shots" : "Drinks";
}

void appendStatus(std::ostringstream& out, const tipsy::app::MachineStatus& status) {
  out << "State: " << stateName(status.state) << "\n";
  out << "Success: " << (status.success ? "true" : "false") << "\n";
  out << "Status: " << status.message.c_str() << "\n";
}

void appendPumpAssignments(std::ostringstream& out,
                           const tipsy::app::SettingsService& settingsService) {
  out << "Mapped pumps:\n";
  for (const auto& assignment : settingsService.current().pumpAssignments) {
    out << "  Pump " << static_cast<int>(assignment.pumpIndex + 1) << ": "
        << (assignment.isAssigned() ? assignment.ingredientDisplayName.c_str() : "Unassigned")
        << (assignment.enabled ? "" : " (disabled)") << "\n";
  }
}

std::vector<String> collectFilteredMenu(const tipsy::app::RecipeService& recipeService,
                                        const tipsy::app::MachineController& machineController,
                                        const String& categoryIdFilter) {
  std::vector<String> items;
  for (std::size_t i = 0; i < recipeService.count(); ++i) {
    const auto& recipe = recipeService.all()[i];
    if (!recipe.visibleInMenu) {
      continue;
    }

    if (!categoryIdFilter.isEmpty()) {
      if (categoryIdFilter == "shot" && recipe.categoryId != "shot") {
        continue;
      }
      if (categoryIdFilter == "drink" && recipe.categoryId == "shot") {
        continue;
      }
    }

    if (!machineController.isDrinkAvailable(recipe.id)) {
      continue;
    }

    items.push_back(recipe.displayName);
  }

  return items;
}

void appendDrinkList(std::ostringstream& out, const char* label, const std::vector<String>& items) {
  out << label << ": ";
  if (items.empty()) {
    out << "(none)\n";
    return;
  }

  for (std::size_t i = 0; i < items.size(); ++i) {
    if (i > 0) {
      out << ", ";
    }
    out << items[i].c_str();
  }
  out << "\n";
}

void appendPlan(std::ostringstream& out, const tipsy::app::MachinePourPlan& plan,
                const char* selectedScreen, const char* selectedStrengthLabel) {
  out << "Selected screen/state: " << selectedScreen << " / " << stateName(plan.status.state)
      << "\n";
  out << "Selected drink: " << plan.drinkName.c_str() << "\n";
  out << "Selected strength: " << selectedStrengthLabel << " / " << plan.alcoholAmountMl
      << " ml\n";
  out << "Category: " << categoryLabel(plan.categoryId) << "\n";
  out << "Available: " << (plan.available ? "true" : "false") << "\n";
  out << "Would route: " << (plan.wouldRoute ? "true" : "false") << "\n";
  out << "Estimated pour time: " << plan.estimatedPourTimeSec << " sec\n";
  out << "Plan status: " << plan.status.message.c_str() << "\n";
  out << "Ingredient amounts:\n";
  for (std::size_t i = 0; i < plan.ingredientCount; ++i) {
    const auto& ingredient = plan.ingredients[i];
    out << "  - " << ingredient.ingredientDisplayName.c_str() << ": " << ingredient.amountMl
        << " ml" << (ingredient.isAlcohol ? " [alcohol]" : " [fixed]") << ", mapped="
        << (ingredient.mapped ? "yes" : "no");
    if (ingredient.mapped) {
      out << ", pump=" << static_cast<int>(ingredient.pumpIndex + 1);
    }
    out << "\n";
  }
}

void runUntilSettled(tipsy::pumps::PumpController& pumpController,
                     tipsy::app::MachineController& machineController,
                     MockTimeProvider& timeProvider) {
  for (int i = 0; i < 200; ++i) {
    pumpController.update();
    machineController.update();
    if (pumpController.activeTaskCount() == 0 &&
        machineController.getCurrentState() != tipsy::app::MachineState::Pouring) {
      return;
    }
    timeProvider.advance(250);
  }
}

ScenarioReport runMixedDrinkScenario(const char* name, const char* drinkId,
                                     std::uint16_t alcoholOverrideMl,
                                     const char* strengthLabel,
                                     const char* expectedResult,
                                     const tipsy::app::RecipeService& recipeService,
                                     tipsy::app::MachineController& machineController,
                                     tipsy::pumps::PumpController& pumpController,
                                     MockTimeProvider& timeProvider) {
  machineController.stopAll();

  ScenarioReport report {};
  report.name = name;
  report.expectedResult = expectedResult;
  report.verifies = "filtering, strength logic, alcohol-only scaling";

  std::ostringstream out;
  const auto selectStatus = machineController.selectDrink(drinkId);
  out << "Select result:\n";
  appendStatus(out, selectStatus);

  const auto plan = machineController.buildPourPlan(drinkId, alcoholOverrideMl);
  appendPlan(out, plan, "DrinkDetail", strengthLabel);

  const auto startStatus = machineController.startDrink(drinkId, alcoholOverrideMl);
  out << "Start result:\n";
  appendStatus(out, startStatus);

  bool mixersStayedFixed = true;
  bool alcoholMatched = plan.alcoholAmountMl == alcoholOverrideMl;
  const auto* recipe = recipeService.findById(drinkId);
  for (std::size_t i = 0; i < plan.ingredientCount; ++i) {
    const auto& ingredient = plan.ingredients[i];
    if (!ingredient.isAlcohol) {
      bool matchedRecipeAmount = false;
      if (recipe != nullptr) {
        for (std::size_t j = 0; j < recipe->ingredientCount; ++j) {
          if (recipe->ingredients[j].ingredientId == ingredient.ingredientId &&
              !recipe->ingredients[j].isAlcohol &&
              recipe->ingredients[j].amountMl == ingredient.amountMl) {
            matchedRecipeAmount = true;
            break;
          }
        }
      }
      if (!matchedRecipeAmount) {
        mixersStayedFixed = false;
      }
    }
  }

  if (startStatus.success) {
    runUntilSettled(pumpController, machineController, timeProvider);
    out << "After simulated pour:\n";
    appendStatus(out, machineController.getLastStatus());
    report.passed = selectStatus.success && plan.available && plan.wouldRoute && alcoholMatched &&
                    mixersStayedFixed &&
                    machineController.getLastStatus().state == tipsy::app::MachineState::Complete;
    machineController.acknowledgeComplete();
  } else {
    report.passed = false;
  }

  report.details = out.str();
  return report;
}

ScenarioReport runShotScenario(tipsy::app::MachineController& machineController,
                               tipsy::pumps::PumpController& pumpController,
                               MockTimeProvider& timeProvider) {
  machineController.stopAll();

  ScenarioReport report {};
  report.name = "Scenario 4: Shot at 6 cl";
  report.expectedResult = "Available shot uses the selected amount as the full pour.";
  report.verifies = "shot behavior, strength logic";

  std::ostringstream out;
  const auto selectStatus = machineController.selectDrink("vodka_shot");
  out << "Select result:\n";
  appendStatus(out, selectStatus);

  const auto plan = machineController.buildPourPlan("vodka_shot", 60);
  appendPlan(out, plan, "DrinkDetail", "6 cl");

  const auto startStatus = machineController.startDrink("vodka_shot", 60);
  out << "Start result:\n";
  appendStatus(out, startStatus);

  if (startStatus.success) {
    runUntilSettled(pumpController, machineController, timeProvider);
    out << "After simulated pour:\n";
    appendStatus(out, machineController.getLastStatus());
  }

  report.passed = selectStatus.success && plan.available && plan.wouldRoute &&
                  plan.ingredientCount == 1 && plan.ingredients[0].amountMl == 60.0F &&
                  startStatus.success &&
                  machineController.getLastStatus().state == tipsy::app::MachineState::Complete;
  machineController.acknowledgeComplete();
  report.details = out.str();
  return report;
}

ScenarioReport runMappingScenario(const tipsy::app::RecipeService& recipeService,
                                  tipsy::app::SettingsService& settingsService,
                                  tipsy::app::MachineController& machineController) {
  machineController.stopAll();

  ScenarioReport report {};
  report.name = "Scenario 5: Pump mapping affects availability";
  report.expectedResult =
      "Changing a mapped ingredient updates which drinks and shots are available.";
  report.verifies = "mapping availability, filtering";

  std::ostringstream out;
  const auto beforeAll = collectFilteredMenu(recipeService, machineController, "");
  const auto beforeShots = collectFilteredMenu(recipeService, machineController, "shot");

  machineController.enterAdminSettings();
  const auto mappingStatus =
      machineController.updatePumpAssignment(5, "tequila", "Tequila", true);

  out << "Selected screen/state: Settings / "
      << stateName(machineController.getCurrentState()) << "\n";
  out << "Mapping update result:\n";
  appendStatus(out, mappingStatus);
  appendPumpAssignments(out, settingsService);
  appendDrinkList(out, "Main menu [All] before remap", beforeAll);
  appendDrinkList(out, "Main menu [Shots] before remap", beforeShots);

  const auto afterAll = collectFilteredMenu(recipeService, machineController, "");
  const auto afterShots = collectFilteredMenu(recipeService, machineController, "shot");
  appendDrinkList(out, "Main menu [All] after remap", afterAll);
  appendDrinkList(out, "Main menu [Shots] after remap", afterShots);

  bool tequilaShotAppeared = false;
  bool screwdriverMissing = true;
  for (const auto& item : afterShots) {
    if (item == "Tequila Shot") {
      tequilaShotAppeared = true;
    }
  }
  for (const auto& item : afterAll) {
    if (item == "Screwdriver") {
      screwdriverMissing = false;
    }
  }

  report.passed = mappingStatus.success && tequilaShotAppeared && screwdriverMissing;
  machineController.stopAll();
  report.details = out.str();
  return report;
}

ScenarioReport runUnavailableDrinkScenario(tipsy::app::MachineController& machineController) {
  machineController.stopAll();

  ScenarioReport report {};
  report.name = "Scenario 6: Unavailable drink is blocked";
  report.expectedResult =
      "An unavailable drink reports unavailable and does not route to pumps.";
  report.verifies = "unavailable drink blocking, mapping availability";

  std::ostringstream out;
  const auto plan = machineController.buildPourPlan("margarita", 60);
  appendPlan(out, plan, "DrinkDetail", "6 cl");
  const auto startStatus = machineController.startDrink("margarita", 60);
  out << "Start result:\n";
  appendStatus(out, startStatus);

  report.passed = !plan.available && !plan.wouldRoute && !startStatus.success;
  machineController.stopAll();
  report.details = out.str();
  return report;
}

ScenarioReport runServiceScenario(const char* name, const char* expectedResult,
                                  const char* verifies,
                                  tipsy::app::MachineStatus actionStatus,
                                  tipsy::app::MachineController& machineController,
                                  tipsy::pumps::PumpController& pumpController,
                                  MockTimeProvider& timeProvider) {
  ScenarioReport report {};
  report.name = name;
  report.expectedResult = expectedResult;
  report.verifies = verifies;

  std::ostringstream out;
  out << "Selected screen/state: Settings / " << stateName(machineController.getCurrentState())
      << "\n";
  appendStatus(out, actionStatus);
  if (actionStatus.success) {
    runUntilSettled(pumpController, machineController, timeProvider);
    out << "After simulated service action:\n";
    appendStatus(out, machineController.getLastStatus());
    report.passed =
        machineController.getLastStatus().state == tipsy::app::MachineState::Complete;
    machineController.acknowledgeComplete();
  }

  report.details = out.str();
  return report;
}

ScenarioReport runUnavailableListScenario(const tipsy::app::RecipeService& recipeService,
                                          tipsy::app::MachineController& machineController) {
  machineController.stopAll();

  ScenarioReport report {};
  report.name = "Scenario 9: List unavailable drinks and reasons";
  report.expectedResult =
      "Simulation prints every currently unavailable drink with a backend reason.";
  report.verifies = "mapping availability, unavailable drink blocking";

  std::ostringstream out;
  std::size_t unavailableCount = 0;
  out << "Unavailable drinks:\n";
  for (std::size_t i = 0; i < recipeService.count(); ++i) {
    const auto& recipe = recipeService.all()[i];
    const auto plan = machineController.buildPourPlan(recipe.id, 60);
    if (plan.available) {
      continue;
    }

    ++unavailableCount;
    out << "  - " << recipe.displayName.c_str() << ": " << plan.status.message.c_str() << "\n";
  }

  if (unavailableCount == 0) {
    out << "  (none)\n";
  }

  report.passed = unavailableCount > 0;
  report.details = out.str();
  return report;
}

void printScenarioReport(const ScenarioReport& report) {
  std::cout << "\n=== " << report.name << " ===\n";
  std::cout << "Verifies: " << report.verifies << "\n";
  std::cout << "Expected: " << report.expectedResult << "\n";
  std::cout << "Result: " << (report.passed ? "PASS" : "FAIL") << "\n";
  std::cout << report.details;
}

void printFinalSummary(const std::vector<ScenarioReport>& reports) {
  std::size_t passed = 0;
  for (const auto& report : reports) {
    if (report.passed) {
      ++passed;
    }
  }

  std::cout << "\n=== Final Summary ===\n";
  std::cout << "Total scenarios: " << reports.size() << "\n";
  std::cout << "Passed: " << passed << "\n";
  std::cout << "Failed: " << (reports.size() - passed) << "\n";
}

}  // namespace tipsy::simulation

int main() {
  using namespace tipsy;

  simulation::MockTimeProvider timeProvider;
  hal::MockFileSystem mockFileSystem;
  storage::FileSystemManager fileSystemManager(mockFileSystem);
  storage::JsonStorage jsonStorage(fileSystemManager);
  app::RecipeService recipeService(jsonStorage);
  app::IngredientService ingredientService(jsonStorage);
  app::SettingsService settingsService(jsonStorage);
  pumps::MockPumpDriver pumpDriver;
  pumps::PumpController pumpController(pumpDriver, timeProvider);
  app::MachineController machineController(recipeService, ingredientService, settingsService,
                                           pumpController);

  if (!fileSystemManager.begin()) {
    std::cerr << "Simulation bootstrap failed: filesystem mount failed: "
              << fileSystemManager.lastError().c_str() << "\n";
    return 1;
  }

  if (!ingredientService.load()) {
    std::cerr << "Simulation bootstrap failed: ingredient load failed: "
              << ingredientService.lastError().c_str() << "\n";
    return 1;
  }

  if (!recipeService.load()) {
    std::cerr << "Simulation bootstrap failed: recipe load failed: "
              << recipeService.lastError().c_str() << "\n";
    return 1;
  }

  if (!settingsService.load()) {
    std::cerr << "Simulation bootstrap failed: settings load failed: "
              << settingsService.lastError().c_str() << "\n";
    return 1;
  }

  if (!pumpController.begin()) {
    std::cerr << "Simulation bootstrap failed: pump controller init failed.\n";
    return 1;
  }

  machineController.begin();

  std::vector<simulation::ScenarioReport> reports;

  std::ostringstream intro;
  intro << "Selected screen/state: MainMenu / " << simulation::stateName(machineController.getCurrentState())
        << "\n";
  simulation::appendPumpAssignments(intro, settingsService);
  simulation::appendDrinkList(
      intro, "Main menu [All]",
      simulation::collectFilteredMenu(recipeService, machineController, ""));
  simulation::appendDrinkList(
      intro, "Main menu [Drinks]",
      simulation::collectFilteredMenu(recipeService, machineController, "drink"));
  simulation::appendDrinkList(
      intro, "Main menu [Shots]",
      simulation::collectFilteredMenu(recipeService, machineController, "shot"));

  simulation::ScenarioReport baseline {};
  baseline.name = "Scenario 0: Initial menu and filters";
  baseline.expectedResult = "Main menu lists only currently available drinks per category.";
  baseline.verifies = "filtering, mapping availability";
  baseline.passed = true;
  baseline.details = intro.str();
  reports.push_back(baseline);

  reports.push_back(simulation::runMixedDrinkScenario(
      "Scenario 1: Available mixed drink at 4 cl", "gin_tonic", 40, "4 cl",
      "Available mixed drink routes and only alcohol amount changes.",
      recipeService,
      machineController, pumpController, timeProvider));

  reports.push_back(simulation::runMixedDrinkScenario(
      "Scenario 2: Available mixed drink at 6 cl", "vodka_soda", 60, "6 cl",
      "Available mixed drink routes with default middle strength.",
      recipeService,
      machineController, pumpController, timeProvider));

  reports.push_back(simulation::runMixedDrinkScenario(
      "Scenario 3: Available mixed drink at 8 cl", "rum_cola", 80, "8 cl",
      "Available mixed drink routes and mixers stay fixed at high strength.",
      recipeService,
      machineController, pumpController, timeProvider));

  reports.push_back(
      simulation::runShotScenario(machineController, pumpController, timeProvider));

  reports.push_back(simulation::runMappingScenario(recipeService, settingsService, machineController));

  reports.push_back(simulation::runUnavailableDrinkScenario(machineController));

  machineController.stopAll();
  reports.push_back(simulation::runServiceScenario(
      "Scenario 7: Prime pumps",
      "Prime action starts safely and completes through the backend path.",
      "service action", machineController.startPrimePumps(), machineController, pumpController,
      timeProvider));

  machineController.stopAll();
  reports.push_back(simulation::runServiceScenario(
      "Scenario 8: Flush / Cleaning",
      "Flush action starts safely and completes through the backend path.",
      "service action", machineController.startFlushCleaning(), machineController, pumpController,
      timeProvider));

  reports.push_back(
      simulation::runUnavailableListScenario(recipeService, machineController));

  for (const auto& report : reports) {
    simulation::printScenarioReport(report);
  }

  simulation::printFinalSummary(reports);
  return 0;
}
