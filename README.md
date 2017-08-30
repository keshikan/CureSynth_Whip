# MIDI Synthesizer for STM32F7 "CureSynth Whip"

## Overview

This repository shows STM32F7 (ARM Cortex-M7) based polyphonic MIDI synthesizer "CureSynth Whip", which is GM1(General MIDI Level 1) nearly compatible.
It can generate sound about 128 Instruments and 1 Drum set. Delay, LPF/HPF, Distortion effects are available for each part.

STM32F7を使用した、16パートのマルチティンバーMIDI音源「CureSynth Whip」です。シンセサイザー部はGM1規格に(ほぼ)対応しており、メロディ128音色、ドラムセット1種のパッチリストを搭載しています。各パートに対し、ディレイ・LPF/HPF・ディストーションを掛けることができます。

* [Demo movie on YouTube](https://youtu.be/-Fs1h-mr3lg)
* [Demo track on SoundCloud](https://soundcloud.com/keshikan/reach)

![Sample Image](http://www.keshikan.net/img/curesynth_github.png)

## Specifications

### Hardware spec

* **MCU:** STM32F765VIT6 / ARM Cortex-M7 (STMicroelectronics)
* **DAC:** PCM5102A (Texas Instruments)
* **Display:** 128*64 SH1106 OLED Module (No brand, bought from AliExpress)

### Synthesizer spec

* **Sound generation:** One oscillator(sin, saw, square, triangle, noise) + one ring-modulator
* **Max polyphony:** 36
* **Number of channels:** 16 (one MIDI-IN)
* **Output:** 16bit, 32kHz
* **Effects:** Delay, LPF/HPF, Distortion, Sweep(Up, Down)
* **Format:** General MIDI 1, Original format

## Building Information

### Making hardware

See [Schematic](./hardware/schematic.pdf).

### Building software

Install [SW4STM32](http://www.openstm32.org/HomePage)(need registration), import [Project files](./software/SW4STM32_project/), and build.  
In addition, use [this binary file for STM32F765](./software/bin).

*Notice: Use gcc optimization flag "-O3"*.

## File Location

* [Binary file for STM32F765](./software/bin/)
* [STM32CubeMX Project file](./software/SW4STM32_project/CureSynth_Whip.ioc)
* [Source codes and project files (System WorkBench for STM32)](./software/SW4STM32_project/)
* [Schematic](./hardware/schematic.pdf)

## Memos for Development (written in Japanese)

* [STM32F7で作る自作MIDI音源](http://www.keshikan.net/gohantabeyo/?p=283)
  * [その1 開発用ボードの設計](http://www.keshikan.net/gohantabeyo/?p=207)
  * [その2 音源部の構成](http://www.keshikan.net/gohantabeyo/?p=261)
  * [その3 ソフトウェアの概要](http://www.keshikan.net/gohantabeyo/?p=301)
  * [その4 MIDIメッセージの受信とリングバッファ](http://www.keshikan.net/gohantabeyo/?p=301)

## References

### MIDI specification

* [The Complete MIDI 1.0 Detailed Specification](https://www.midi.org/specifications/item/the-midi-1-0-specification) ([MIDI Association](https://www.midi.org/)
* [MIDI1.0規格書](http://amei.or.jp/midistandardcommittee/MIDI1.0.pdf) ([AMEI](http://amei.or.jp/))
* [MIDIの学習](http://www1.plala.or.jp/yuuto/midi/index.html) ([Laboratory "U"](http://www1.plala.or.jp/yuuto/top.html))

### Synthesizer programming

* DDS Function Generator ([English](http://elm-chan.org/works/asg/report_e.html), [Japanese](http://elm-chan.org/works/asg/report_j.html)) ([ELM by ChaN](http://elm-chan.org/))
* [シンセプログラミング](http://www.geocities.jp/daichi1969/synthprog/index.html) ([Daichi Laboratory](http://www.geocities.jp/daichi1969/index.html))
* [音程のデータを求める](http://park19.wakwak.com/~gadget_factory/factory/miditalk/tone.html) ([Gadget Factory](http://park19.wakwak.com/~gadget_factory/index.html))

### Digital filter programming

* [Plotting RBJ Audio-EQ-Cookbook HPF/LPF](http://aikelab.net/filter/) ([Aike Lab](http://aikelab.net/))
* [簡単なデジタルフィルタの実装](http://vstcpp.wpblog.jp/?page_id=523) ([C++でVST作り](http://vstcpp.wpblog.jp/))

### Hardware

* 「OPアンプ活用 成功のかぎ」, Akihiro Kawata, CQ Publishing, 2009, ISBN-13:978-4789842068

## Author

(c) 2017 Keshikan ( [Website](http://www.keshikan.net/),  [Twitter](https://twitter.com/keshinomi_88pro) )

## License

Source codes and schematic are licensed under [GPLv3](https://www.gnu.org/licenses/gpl-3.0.html) unless otherwise specified.
