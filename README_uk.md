# RewardsTheater

Плагін до [OBS](https://obsproject.com/), який дозволяє твоїм глядачам замовляти відео та звуки на стримі за бали каналу.

[Подивись це відео](https://youtu.be/-0evZCAlXVU) (якщо знаєш англійську) про те, як усе налаштувати. 
Або можна прочитати повні інструкції нижче.

[Read in English here](README.md)

## Чим він кращий ніж [TR!GGER FYRE](https://overlays.thefyrewire.com/widgets/triggerfyre/)?

- Вільне та відкрите програмне забезпечення.
- Можна використовувати локально збережені відео. Не треба нічого завантажувати на сервер.
- Є гарний інтерфейс для редагування нагород прямо всередині OBS!
- Використовує вбудовані джерела відео OBS замість джерела «Браузер». Це означає більший FPS та значно кращу якість.
- Може відтворювати декілька відео, якщо ти хочеш (просто додай декілька відео у VLC Video Source).
- Замовлення нагород можна поставити в чергу, щоб вони не грали одночасно.

![ui](readme_images/ui_uk.png)

## Вимоги
- Ти Twitch Affiliate або Partner, і в тебе ввімкнені бали каналу.
- OBS 29.1.2 або новішої версії.
- Операційна система Windows або Linux.


## Налаштування
### Установлення на Windows
1. Установи останню версію Visual C++ Redistributable [звідси](https://aka.ms/vs/17/release/vc_redist.x64.exe).
2. Завантаж та запусти exe інсталятор за [цим посиланням](https://github.com/gottagofaster236/RewardsTheater/releases/latest).
3. Замість інсталятора, можна завантажити zip файл та розпакувати його в папку, куди встановлено OBS (зазвичай `C:\Program Files\obs-studio`).
### Установлення на Linux

+ #### Debian/Ubuntu

  Для Ubuntu та інших дистрибутивів, заснованих на Debian, можна завантажити deb файл за [цим посиланням](https://github.com/gottagofaster236/RewardsTheater/releases/latest). Потім установи його наступним чином:
  ```
  sudo dpkg -i /path/to/deb/file
  ```

+ #### Arch Linux

  Доступно у [AUR](https://wiki.archlinux.org/index.php/Arch_User_Repository) як пакунки **[rewards-theater-obs](https://aur.archlinux.org/packages/rewards-theater-obs)** (стабільна) та **[rewards-theater-obs-git](https://aur.archlinux.org/packages/rewards-theater-obs-git)**.

+ #### Flathub

  Доступно як Flatpak [тут](https://flathub.org/apps/com.obsproject.Studio.Plugin.RewardsTheater). Якщо у вашого дистрибутиву є підтримка [flatpak](https://flathub.org), і у вас увімкнений [репозиторій flathub](https://flatpak.org/setup/):

  ```
  flatpak install flathub com.obsproject.Studio.Plugin.RewardsTheater
  ```

### Додавання джерела
1. RewardsTheater підтримує відео і звуки. Інструкції для цього аналогічні.
2. Додай джерело OBS для кожного відео, яке те хочеш додати як нагороду за бали каналу. Підтримуються «Джерело мультимедіа» або «Джерело відео VLC»:
   
   ![source](readme_images/source_uk.png)
3. Вибери файл із відео зі свого комп'ютера. Якщо ти хочеш, щоб відео з'являлося та зникало плавно, вимкни галочку «Не показувати джерело, коли відтворення завершено», і вибери переходи (показати/сховати) для джерела:
   
   ![media_source](readme_images/media_source_uk.png)
   ![transition](readme_images/transition_uk.png)
4. Розташуй джерело на сцені так, як тобі хочеться.

### Додавання джерела з випадковим відео
Ти можеш використовувати VLC Video Source з декількома відео в плейлисті. Тоді RewardsTheater щоразу відтворюватиме випадкове відео з цього плейлисту.

Зверни увагу, що в такому разі переходи для зникнення джерела не підтримуються.

### Створення нагороди

1. Натисни Інструменти → RewardsTheater.
   
   ![tools](readme_images/tools_uk.png)
2. Спочатку треба ввійти у Twitch, натиснувши на «Увійти».
3. Потім треба натиснути на «Додати нагороду». Не забудь додати джерело, яке було створено раніше, як джерело мультимедіа для нагороди. **Натисни на «Перевірити джерело», щоб перевірити, чи працюватиме джерело.** Через обмеження API Twitch, завантажити свою іконку можна тільки в браузері.
   
   ![add_reward](readme_images/add_reward_uk.png)

7. Згодом можна ще раз відредагувати нагороду, натиснувши на неї в меню Інструменти → RewardsTheater.

   ![settings](readme_images/settings_uk.png)

8. Якщо нагород багато, можна помістити джерела мультимедіа на окрему сцену, і додати цю сцену на інші сцени.

### Відстеження нагород
1. Під час стриму можна стежити за чергою нагород у разі, якщо одночасно є багато замовлень. Можна також скасувати замовлення, натиснувши на хрестик — тоді бали каналу буде повернуто відповідному глядачу.

   ![rewards_queue](readme_images/rewards_queue_uk.png)

2. Можна натиснути на «Призупинити відтворення нагород» на екрані з налаштуваннями, якщо ти не хочеш, щоб відео відтворювалися певний час. У цей час глядачам також будуть повертатися їхні бали.

## Підтримати розробника
Якщо не складно, постав зірку на GitHub репозиторій 🙂

## Building
PRs are welcome! If you want to build RewardsTheater yourself, please refer to [BUILDING.md](BUILDING.md)

## License and credits
- RewardsTheater is licensed under GNU General Public License v3.0. 
- RewardsTheater is a plugin to [OBS Studio](https://github.com/obsproject/obs-studio), which is licensed under GNU General Public License v2.0 or later.
- RewardsTheater uses [Boost.Asio](https://www.boost.org/doc/libs/1_83_0/doc/html/boost_asio.html), [Boost.URL](https://www.boost.org/doc/libs/1_83_0/libs/url/doc/html/index.html), [Boost.Beast](https://www.boost.org/doc/libs/1_83_0/libs/beast/doc/html/index.html), [Boost.JSON](https://www.boost.org/doc/libs/1_83_0/libs/json/doc/html/index.html), which are licensed under the Boost Software License, Version 1.0.
- RewardsTheater uses [Qt Core](https://doc.qt.io/qt-6/qtcore-index.html), [Qt Widgets](https://doc.qt.io/qt-6/qtwidgets-index.html) and [Qt GUI](https://doc.qt.io/qt-6/qtgui-index.html) modules, which are available under GNU General Public License v2.0 or later.
- RewardsTheater uses [OpenSSL](https://openssl.org/), which is licensed under Apache-2.0 License.
- RewardTheater uses Google's [material-design-icons](https://github.com/google/material-design-icons/tree/master), which are licensed under Apache-2.0 license.
