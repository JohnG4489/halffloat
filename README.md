## Objectif / Purpose

### 🇫🇷 Version française

L'objectif de cette bibliothèque est de présenter une implémentation **half-float (FP16)** de manière **purement algorithmique**, conforme à la norme **IEEE-754-2008 / 2019**, sans aucune optimisation spécifique au matériel.
Elle respecte les spécifications du format **binaire16**, y compris la gestion des **NaN**, des **valeurs infinies**, des **±0** et l'**ordre total IEEE** pour les comparaisons ("fmin", "fmax", etc.).

Le code est écrit en **C99**, tout en respectant plusieurs **rigueurs du C90** afin de garantir la **portabilité** et la **lisibilité** du code.
Les algorithmes sont simples, clairs et facilement **transposables en assembleur 6809 ou 68000**, voire sur **FPGA**.
Chaque fonction est **structurée de façon logique** : la **gestion des cas particuliers** est traitée en premier, suivie de la **gestion du cas général**, pour faciliter la lecture et la compréhension du déroulement algorithmique.
L'approche est **déterministe**, adaptée aux environnements **sans unité flottante (FPU)**.
Chaque fonction met l'accent sur la **compréhension de l'algorithme sous-jacent**, plutôt que sur la **performance brute**, afin de servir de **base d'étude, de portage ou d'expérimentation** dans des environnements contraints.

**Note :** les commentaires du code et une **importante partie du code de test** ont été générés à l'aide d'une **intelligence artificielle**, dans un but de documentation et de cohérence stylistique.

---

### 🇬🇧 English version

The purpose of this library is to provide a **purely algorithmic implementation** of the **half-float (FP16)** format, **compliant with the IEEE-754-2008 / 2019 standard**, without any hardware-specific optimization.
It implements the **binary16** specification, including correct handling of **NaNs**, **infinities**, **signed zeros (±0)**, and the **IEEE totalOrder** rules for comparisons ("fmin", "fmax", etc.).

The code is written in **C99**, while preserving several **C90 strictness rules** to ensure **portability** and **readability**.
Algorithms are simple, clear, and easily **translated into 6809 or 68000 assembly**, or implemented on **FPGA**.
Each function follows a **structured logic**: it first handles **special cases**, then the **general case**, making the algorithm easy to read and reason about.
The approach is **deterministic**, suitable for **environments without a floating-point unit (FPU)**.
Each function focuses on **understanding the underlying algorithm** rather than **raw performance**, serving as a **foundation for study, porting, or experimentation** in constrained systems.

**Note:** the code comments and a **significant portion of the test code** were generated using **artificial intelligence**, for documentation and stylistic consistency purposes.