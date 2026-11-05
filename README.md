# SakuraHell (VST3 для FL Studio)

## Крутилки
| Крутилка | Казнь | Звук |
|---|---|---|
| PRESS / CRUSH | пресс давит чела | битдепт + даунсемпл |
| BLADE / HELL | лезвие, голова отлетает | дисторшн + ринг-мод |
| SPIKES / PITCH | шипы из пола | питч -24..+12 полутонов |
| ACID / DOOM | кислота льётся сверху | адский реверб |
| OUTPUT | - | громкость на выходе |

Фон: огромная маска, чем громче сигнал, тем шире улыбка.

## Сборка (Windows)
1. Поставь Visual Studio 2022 (Desktop development with C++), CMake и Git.
2. В папке проекта:
   cmake -B build -G "Visual Studio 17 2022"
   cmake --build build --config Release
3. Готовый SakuraHell.vst3 сам скопируется в C:\Program Files\Common Files\VST3
4. FL Studio: Options > Manage plugins > Find plugins > Start scan.
   Дальше кидай на мастер или канал как эффект.

## Сборка без установки программ (через GitHub)
1. Зарегистрируйся на github.com, создай новый репозиторий (New repository).
2. Нажми "uploading an existing file" и перетащи ВСЁ содержимое папки SakuraHell
   (вместе с папкой .github, она скрытая, включи показ скрытых файлов). Commit.
3. Вкладка Actions > Build SakuraHell. Подожди ~15 минут, пока загорится зелёная галочка.
4. Открой этот запуск, внизу в Artifacts скачай SakuraHell-VST3 (плагин) и SakuraHell-EXE (проверить отдельно).
5. Папку SakuraHell.vst3 положи в C:\Program Files\Common Files\VST3 и просканируй плагины в FL Studio.
