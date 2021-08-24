# xeris-I
XERIS/APEX System I - A very experimental/sci-fi nanokernel distributed OS

## Premise

System I was a **very** experimental project to build an infinitely scalable distributed operating system with a specification-driven design. The premise, and core design-goals, were pretty sci-fi. In some sense, I tried to do as little as possible in a traditional way. This OS was designed conceptually to run literally forever, as in millions of years/rest of the universe/post-humanity. This OS was imagined to, for example, run something like a dyson sphere, or self-replicating bot swarms that colonize the galaxy and beyond.

Each node is autonomous, and will often be small and specialized, or huge and general. Node discovery is autonomous, and nodes can collectively form themselves into a "Tier", which itself appears to be a node. Other tiers will see other tiers at the same level as being nodes, and therefore the system is infinitely recursive. A given tier-level node does not know if it is made up of nodes in a lower-tier, or if it is just a bare-metal thing. In theory, eventually, the entire universe could be a single node of extreme capability, but which physically operates at astronomic timescales.

## Status

This OS did successfully reach first boot on an AVR328P, running a multi-tasking application that both blinked some lights, and managed a terminal.

If you are motivated enough, it should be possible to use it in an AVR32 with sufficient flash, but you're on your own.

System I will no longer be developed by me, and is here really as a personal museum/portfolio piece more than anything.

## Future

This OS is the philosophical successor to XERIS/APEX System II, which is a much less experimental project of similar (but less wild) structure and capabilities. System II is being written from scratch entirely in Ada. A paper on System II can be found [here](https://annexi-strayline.com/papers/AUJ%2042-1-XERIS-APEX_Wai).

System II will be licensed under a BSD 3-clause license, and is managed under ANNEXI-STRAYLINE. The github repo for this is (not yet made public, but will be soon, a which point this will be a link).

