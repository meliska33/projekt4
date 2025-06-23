
# Sprawozdanie: Symulator windy z kolorowymi pasażerami

## Opis projektu

Projekt jest symulatorem windy w języku **C++**, korzystając z biblioteki **GDI+**.
Projekt przewiduje następujące działania:
- Dodawanie pasażerów na poszczególnych piętrach,
- Wizualizację ich ruchu,
- Kolorowe oznaczenie pasażerów według docelowego piętra,
- Logikę poruszania się windy.

## Główne funkcjonalności

- **Piętra**: 4 piętra + parter
- **Pasażerowie**:
  - Każdy pasażer ma swoje piętro startowe oraz piętro docelowe,
  - Kabina ma zdefiniowany maksymalny udźwig, wynoszący 600 kg,
  - Kolory identyfikujące piętro docelowe (legenda znajduje się na dole projektu).
- **Winda**:
  - Możliwość ręcznego wysłania windy na dane piętro,
  - Gdy brak zadań, winda wraca na parter po 5 sekundach bezczynności.

## Interfejs użytkownika

- **Przyciski**: 
  - Górny pasek przycisków: ręczne wysłanie windy na dowolne piętro
  - Przyciski na każdym z pięter: przyzywanie ludzi na danym piętrze, z podziałem na ich piętro docelowe
- **Wizualizacja**:
  - Szyb windy z kabiną,
  - Pasażerowie jako kółka z kolorami odpowiadającymi docelowym piętrom,
  - Informacje tekstowe: aktualne piętro, obciążenie windy,
  - Legenda kolorów.

## Główne funkcje realizujące działanie projektu

- int countPassengersInElevator() -> funkcja zliczająca wagę pasażerów w windzie 
- void addPickupFloor(int floor) -> funkcja odpowiedzalna za aktualizację zbioru pickupFloors (piętra z pasażerami)
- void addDropoffFloor(int floor) -> funkcja odpowiedzalna za aktualizację zbioru dropoffFloors (piętra docelowe)
- void updateTargetFloor() -> funkcja odpowiedzalna za aktualizację piętra, na które ma jechać winda
- void checkIdleState() -> funkcja odpowiedzialna za kontrolę bezczynności windy
- void moveElevatorStep(HWND hwnd) -> funkcja odpowiedzialna za ruch windy

## Technologie

- **Język**: C++
- **Biblioteki**:
  - `WinAPI` – obsługa okien i interakcji.
  - `GDI+` – renderowanie grafiki 2D.
- **Środowisko**: Windows
