Программа выполняет статический анализ `.dll` файлов и ищет сигнатуры (ключевые строки) из приложенного доклада по DLL-Inject.

Сканер:
- ищет `.dll` в папке, где находится сам `.exe`;
- читает DLL как бинарные данные;
- извлекает печатные строки (ASCII и UTF-16LE) как легкий этап "декомпиляции" строк;
- проверяет индикаторы в "сырых" байтах и в извлеченных строках;
- пишет отчет в `scan_report.txt`.

## Что ищет

В сигнатуры включены строки из доклада, в том числе:
- `AxelBB`, `triger`
- `TriggerBot`, `Reach`, `Esp`, `XRay`, `Hitbox`
- `qEXPANDING_BUTTON`, `EXPANDING_BUTTON`, `EXPANDING_BUTTONq`, `GishCode`
- `HARDWARE\DeviceMap\Videoy`, `prevScreenes}`, `WindowsSelectorImpl.javay`
- `axisalignedbb`, `0/qvaxisalignedbb`
- `+compileESP`, `compileESP`, `eKillAura`, `YautoArmor`
- `_execute_onexit_table`, `Module32Next>`, `CreateToolhelp32Snapshot`, `net/minecraft/util/math/AxisAlignedBB`

После запуска рядом с программой появится файл `scan_report.txt`.

## Формат отчета

Для каждого DLL-файла выводится:
- имя файла;
- найденный индикатор;
- `Raw offsets` (смещения в hex, где индикатор найден в байтах);
- `Found in extracted strings` (найдено ли в извлеченном текстовом корпусе).

В конце отчета выводится сводка по числу DLL и совпадений.

## Ограничения

- Это не полноценный дизассемблер и не PE-декомпилятор.
- Поиск основан на сигнатурах и строках, поэтому возможны:
- ложные срабатывания;
- пропуски при сильной обфускации/шифровании строк.
