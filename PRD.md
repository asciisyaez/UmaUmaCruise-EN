# UmaCruise English MVP PRD

## Overview
UmaCruise English is a Windows .NET application that assists players of the global Steam release of *Uma Musume: Pretty Derby* by displaying the effects of in-game event choices. The application reuses the original UmaUmaCruise concept but targets an English environment and focuses on minimal essential features for a first release.

## Goals
- Automatically detect the running game window for the global Steam English release.
- Recognize event screens and choices using Tesseract OCR through GDI screenshot capture.
- Display event choices and their effects in English.
- Load event data from a community-maintained JSON file.
- Provide a minimal, user-friendly interface localized in English.

## Target Users
- English-speaking players of *Uma Musume: Pretty Derby* on Steam seeking quick event guidance.

## Core MVP Features
1. **Game Window Auto-Detection**
   - Continuously scan active windows to find the game and position overlay elements.
2. **OCR Event Recognition**
   - Capture event screens with GDI and run Tesseract OCR tuned for English text.
3. **Event Effect Lookup**
   - Match recognized event names and choice text against a community-sourced JSON database and display corresponding effects.
4. **Event Data Updates**
   - Download the latest event JSON from the community repository and cache it locally.
   - Provide an **Update Events** button that re-fetches the file on demand.
5. **Start/Stop Control**
   - A simple UI with controls to start and stop monitoring, along with basic settings (e.g., toggle auto-detection).

## Data Requirements
- Event metadata is sourced from the community-maintained [UmaUmaCruise-db-urarawin](https://github.com/RyoLee/UmaUmaCruise-db-urarawin) repository.
- The application downloads `UmaMusumeLibrary.json` from `https://raw.githubusercontent.com/RyoLee/UmaUmaCruise-db-urarawin/master/UmaMusumeLibrary.json` on first run and stores it locally.
- The **Update Events** button fetches the latest version of this file and replaces the cached copy.

## Technical Requirements
- .NET 6 or later targeting Windows 10+ 64-bit.
- Tesseract OCR integrated through a GDI-based capture pipeline.
- Signed binaries or Windows SmartScreen-friendly distribution.

## Open Questions
- Preferred distribution method (standalone installer vs. portable zip).
- Process for community contributions to the event JSON.

