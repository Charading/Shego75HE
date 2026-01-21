# Nocturne 75 — How This Keyboard Ended Up Here

I’ve been working on this keyboard for months now, and honestly, it’s gone through *so many* changes that it reached a point where I needed to actually stop and write down why things changed the way they did.

A lot of this came down to the screen, the firmware, and just how disconnected everything felt before.

---

## The Old Setup (And Why It Started to Bug Me)

Originally, Shego was split across two chips.

The **RP2040** did all the keyboard stuff — scanning, Hall sensors, HID — and the **ESP32** was basically just there to run the screen. The ESP handled a GIF player that worked off an SD card, and the two chips talked to each other over UART.

On paper, this was fine.

In reality, it kind of sucked.

The screen never felt like part of the keyboard. It always felt like this extra thing bolted on the side that you talked to indirectly. And that disconnect kept getting more obvious the more features I tried to add.

---

## The SD Card Problem

The SD card setup worked, but it was fragile.

If you wanted to change a GIF:
- Open the keyboard  
- Pull the SD card  
- Copy files  
- Put it back  
- Reassemble everything  

That’s fine once. It’s annoying every time after that.

The ESP would copy the GIF from the SD card into flash so it would persist, which was clever, but it still felt like a workaround rather than a real solution.

What I *wanted* was what other keyboards with screens do:
- Send a GIF directly over USB
- Have it show up immediately
- No physical access required

I just couldn’t get there with the split RP2040 + ESP32 setup.

---

## ESP32 Pico-D4: Looked Perfect, Wasn’t

At some point I found the **ESP32 Pico-D4**, and I really thought this was it.

It’s small, cheap, has integrated flash, no external crystal — perfect for a keyboard PCB.

Except… I could not get it to work.

I couldn’t flash it reliably. I couldn’t even tell if it was alive half the time. In hindsight, a lot of that was probably soldering skill, because this was earlier on and I’ve improved a *lot* since then. But at the time, boards were just getting wasted.

That was kind of the moment where I stopped and thought:

> Why am I fighting this so hard?

---

## Rethinking the Entire Direction

Instead of trying to fix the ESP32 approach again and again, I started questioning the whole architecture.

Why am I running two processors?
Why is the screen separate at all?
Why does this feel more complicated the more I improve it?

So I started looking around for alternatives.

---

## Finding the RP2350B

That’s when I came across the **RP2350 series**, and specifically the **RP2350B**.

This thing basically solved *everything* in one go.

- Eight beautiful ADC channels  
- Enough power to handle a screen properly  
- No need for rare, expensive parts like ADG732s even though I did get them very cheap (around £2 each) from AliExpress
- One processor for everything  

The biggest thing though wasn’t specs — it was the realization that **the screen didn’t need to be separate at all**.

The keyboard *could* just be one system.

---

## Porting Everything (And Why It Was Worth It)

Over the last month or so, I’ve been porting the Shego firmware to the RP2350B and rebuilding the screen logic properly.

I designed a Pico-style RP2350B breakout for testing and ran everything on a breadboard. I didn’t even touch Hall scanning at first because that part was already solid — the real work was making the screen feel native.

Once I brought LVGL in, everything clicked.

The UI finally felt like it belonged to the keyboard.

---

## What the Screen Can Do Now
<img src="\assets\demo_tft.gif" alt="Demo TFT" align="right" width="200" style="margin-left: 15px; margin-bottom: 10px;">
The screen isn’t a gimmick anymore.

It’s a proper interface.

There’s a page system:
- One page is just your main GIF
- Another page shows Spotify info — album art, track name, playtime
- Another page shows live Hall sensor data for individual keys

You can literally press a key and watch its actuation live. Right now it’s ADC values, but once the PCB is finalized and calibration is locked in, that’ll turn into proper distance values.

It feels *connected* in a way the old setup never did.

---

## The New GIF System (No More SD Card Pain)

This is probably one of the biggest quality-of-life improvements.

There’s now a **slot-based GIF system in flash**.

- 16 MB external flash
- Slot 0 is the active GIF
- About 10 other slots for saved GIFs

You upload GIFs over USB HID now. No SD card required.

If you *do* want to use the SD card, it’s still there — but now you can browse it from the keyboard, copy files into flash, and manage storage without opening the case.

Changing a GIF is instant.

That alone fixes one of my biggest long-term annoyances with Shego.

---

## Input While Prototyping

Right now I’m using a little 5-button joystick module on the breadboard because it’s just faster for development.

In the real keyboard:
- Navigation will be done with keycodes
- The encoder knob will handle menus
- No extra hardware needed

The important part is that the logic is already there.

---

## Stability Testing (And Breaking It on Purpose)

I’ve spent a lot of time trying to break this.

Streaming live ADC data while LVGL is rendering.
Hammering USB while the screen updates.
Doing things I probably shouldn’t be doing.

It’s held up really well.

I did hit a weird bug where the screen was selecting the wrong key while the software side was correct — turned out to be an index offset issue, which is now fixed.

Overall, it feels solid.

---

## Flashing and Development Now

Moving to the RP2350B made development *so* much easier.

Flashing over USB or SWD.
SWD is way faster now that the firmware is bigger.
Debugging is cleaner.

At this point, once the PCB is done, it’s basically:
- Firmware updates
- Bug fixes
- Feature additions

No more architectural rework.

---

## Lucky65 v2 & v3 Side Projects

Alongside all this, I’ve also designed **drop-in replacement PCBs** for the **Lucky65 v2 and v3**.

The Mina65 and Taki65 are direct replacements — no case mods — and they turn those boards into **Hall Effect keyboards** running the *same* firmware as Shego, just without the screen.

Same scanning logic.
Same actuation model.
Same software quality.

The stock Lucky65 firmware ecosystem is… not great. These boards basically free the hardware from that whole situation and give it a proper firmware stack.

---

## About the Repo

All of this has been developed in a **private repository**.

That was intentional. I wanted to be able to refactor aggressively, break things, and clean stuff up without worrying about public stability.

The codebase is now getting to the point where:
- The architecture is solid
- The direction is locked in
- It’s actually maintainable

I’ll be releasing it publicly soon, once documentation and cleanup are done.

---

## Where This Leaves Nocturne 75

Nocturne 75 feels like the point where everything finally came together.

One processor.
One system.
Screen, firmware, and hardware all designed together instead of fighting each other.

From here on out, it’s refinement — not reinvention.
