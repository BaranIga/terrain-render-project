Renderowanie terenu z danych SRTM (DTED)


Opis

  Program renderuje rzeczywistą rzeźbę terenu na podstawie danych wysokościowych SRTM zapisanych w plikach HGT.
  Aplikacja wczytuje kafelki terenu z podanego katalogu i umożliwia wyświetlanie:
    widoku mapy 2D (rzut równoległy),
    realistycznego widoku 3D (teren na powierzchni kuli Ziemi).
  Program na bieżąco wyświetla:
    liczbę klatek na sekundę (FPS),
    liczbę renderowanych trójkątów na sekundę.
  Renderowanie wykorzystuje OpenGL, rysowanie indeksowane (glDrawElements) oraz mechanizm LOD (Level of Detail) zwiększający wydajność.

Wymagania
  Linux
  OpenGL 3.3+
  GLFW
  GLEW / GLAD
  kompilator C++17
  make

Kompilacja
  Kompilacja programu: make

  Powstaje plik wykonywalny:  ./terrain

Uruchamianie
  Podstawowe uruchomienie (ładowanie wszystkich kafelków z katalogu): ./terrain ./data/

Uruchomienie z zakresem współrzędnych:
  ./terrain ./data/ -lon 16 17 -lat 50 51
gdzie:
  -lon a b – zakres długości geograficznych,
  -lat a b – zakres szerokości geograficznych,

Sterowanie
  Widok mapy (2D)
    przeciąganie myszy – przesuwanie mapy,
    scroll – zoom,
    1–3 – zmiana poziomu LOD,
  Widok 3D
    W A S D – ruch obserwatora,
    mysz – obrót kamery,
    SPACE– przełączanie widok 2D / 3D.

Dane
  Program wczytuje pliki SRTM HGT (1201×1201 próbek).
  Każda próbka to 16-bitowa liczba całkowita (big-endian) określająca wysokość w metrach.
  Wartości oznaczające brak danych są pomijane podczas renderowania.

