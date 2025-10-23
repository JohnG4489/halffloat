## Objectif / Purpose

### FR Version française

L'objectif de cette bibliothèque est de présenter une implémentation **half-float (FP16)** de manière **purement algorithmique**, sans aucune optimisation spécifique au matériel.

Le code est volontairement écrit en **C90**, avec des algorithmes simples pouvant être directement **transcrits en assembleur 6809 ou 68000**.  
L'approche est **déterministe**, adaptée aux environnements **sans unité flottante**.

**Note :** les commentaires du code et une partie du code de test ont été générés à l'aide d'une **intelligence artificielle**, dans un but de documentation et de cohérence stylistique.

---

### UK English version

The purpose of this library is to provide a **purely algorithmic implementation** of the **half-float (FP16)** format, without any hardware-specific optimization.

The code is intentionally written in **C90**, using simple algorithms that can be directly **translated into 6809 or 68000 assembly**.  
The approach is **deterministic** and designed for environments **without a floating-point unit (FPU)**.

**Note:** the code comments and part of the test code were generated using **artificial intelligence**, for documentation and stylistic consistency purposes.
