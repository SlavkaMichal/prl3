---
lang: sk
title: PRL Projekt 3 - Viditeľnosť
author: Michal Slavka
...


### Rozbor algoritmu

* Teoretická časová zložitosť:

    $$ t(n) = O (Up-sweep) + O (Down-sweep) $$

    $$ t(n) = O (log(n)) + O (log(n)) = O (log(n))  $$

    Algoritmus má logaritmickú časovú zložitosť, pretože simuluje priechod
    binárnym stromom od listom ku koreňu a späť, pričom výška vyváženého binárneho stromu
    je logaritmus so základom 2 z počtu listov.

    Toto platí za predpokladu, že počet procesov je rovný počtu prvkov.


* Priestorová zložitosť:
    $$ p(n) = O (n) $$

    Počet potrebných procesorov je $n$ a teda asymptotická zložitosť je $O(n)$.

* Celková cena:
    $$ c(n) = t(n) * p(n) = O(log(n)) * O(n) = O(n*log(n)) $$

* Optimálnosť algoritmu:
    $$ t(n) / c(n) = O (n*log(n)) / O (n) > 1 $$

    Algoritmus je neoptimálny

-----

### Implementacia

Moja implementácia simuluje priechod stromom, a pozostáva z dvoch častí
<em>Up-sweep</em> a <em>Down-sweep</em>. Obe časti sú implementováne sekvenčne
aj paralelne. Strom sa mapuje na pole hodnôt, teda ide o <em>in-place</em>
algoritmus.

Sekvenčné časti algoritmu som implementoval rekurzívne, aby bola implementácia
čo najjednoduchšia.

Paralelný algoritmus delí procesory na dve skupiny, procesory na dve skupiny,
podľa toho, či v danej iterácii sú aktívne alebo nie. Aktívne procesory sú
vždy v pároch, jeden je ľavý a druhý pravý, čo určuje smer komunikácie.

#### Počet procesorov

Počet procesorov je buď rovnaký ako počet vstupných prvkov. V tomto prípade algoritmus
beží úplne paralelne.

V prípade, že počet dostupných procesorov je nižší ako počet vstupných prvkov,
tak sa prvky tak aby na každom procesore bol rovnaký počet prvkov a počet
prvkov na procesore je vždy mocninou dvojky, aby sa potom v paralelnej časti
mohlo pracovať iba s koreňom získaným v sekvenčnej časti. Výnimkou v počte
pridelených prvkov na procesor má posledný procesor, ktorému je pridelený
nižší alebo rovný počet prvkov ako ostatným procesorom.

Strop procesov pri spúšťaní je nastavený na 25, teda pri 26 vstupoch sa použije
iba 13 procesov, každý s dvoma prvkami a zvyšných 12 procesov je ukončených.

-----

### Experimenty

Čas výpočtu som meral od <em>Up-sweep</em> po zozbieranie výsledkov. Merania
boli vykonané na merlinovi a počet procesov bol obmedzený na 25.

Experiment som opakoval 20 krát pre všetky dĺžky vstupov od 2 do 30, plus bod
z ktorého je výška pozorovaná.

Meral som čas jedného vlakna pomocou funkcie <em>MPI_Wtime</em>.

![x osa značí počet procesorov, y osa značí čas. Časy su
spriemerovane z 20 behov.](measurement.png)

\newpage

-----

### Sekvenčný diagram komunikácie

Zjednodušený sekvenčný diagram prípadu v ktorom je 8 procesorov.

![V zovšeobecnení na párne a nepárne procesory som zanedbal existenciu
posledního procesoru, ktorý nemože komunikovať so svojim susedom.](diagram.png)


-----

### Záver

Keďže merania boli robené na merlinovi, je v nich šumu ale mohlo by ísť o
logaritmickú závislosť.
