# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

C++20によるフォルマント音声合成器。日本語の全音素（母音・子音・濁音・半濁音・拗音・促音・撥音）を合成し、WAVファイルとして出力する。最終目標は約500行のコンパクトな単一ファイル実装。

**現在の進捗**: MS1-MS6 完了（formant.cpp ~437行）。5母音 + 20子音（破擦音・全摩擦音含む） /h,s,ɕ,ç,ɸ,z,ʑ,n,m,ɾ,j,w,p,b,t,d,k,g,tɕ,ts,dʑ,dz/ + 促音・撥音を合成可能。ひらがなテキスト入力対応（`./formant "こんにちは"`）。テストスイート完備（19スイート / 176ケース）。

## Build & Run

```bash
# 直接コンパイル
clang++ -std=c++20 -O2 -Wall -o formant formant.cpp

# または Makefile
make

# 実行・再生（ひらがなテキスト入力）
./formant "こんにちは"    # output.wav を生成
./formant                 # デフォルト: "あいうえお"
afplay output.wav         # macOS で再生

# テスト実行（19スイート / 176テストケース）
make test
```

## Architecture

### 合成方式（Klatt簡易版）

```
音源（インパルス列 or ノイズ or 混合）
  → 2次IIR共振器（Resonator）3段直列（F1/F2/F3）
  → WAV出力（16bit/44100Hz/mono）
```

### クラス設計（データ指向 + 薄いクラス）

| 名前 | 種別 | 状態 | 役割 |
|------|------|------|------|
| `SourceType` | enum class | 実装済 | 音源種別: `Impulse`, `Noise`, `Mixed` |
| `FormantParams` | struct (POD) | 実装済 | F1-F3, BW1-BW3, gain, source |
| `Resonator` | struct | 実装済 | 2次IIR共振器。z1,z2 + set() + process() + reset() |
| `NoiseGen` | struct | 実装済 | LCG乱数による白色雑音生成 |
| `ImpulseTrain` | struct | 実装済 | 位相累積型インパルス列生成 |
| `PhonemeEntry` | struct | 実装済 | FormantParams + duration_samples のペア |
| `lerp` | 関数 | 実装済 | 線形補間ヘルパー |
| `ms2s` | 関数 | 実装済 | ミリ秒→秒変換ヘルパー |
| `append` | 関数 | 実装済 | サンプル列追記ヘルパー |
| `makePlosiveCV` | 関数 | 実装済 | 破裂音・破擦音CV音節の生成ヘルパー（closure→burst→VOT/frication→母音） |
| `writeWav` | 関数 | 実装済 | WAVヘッダ+データ書き出し（16bit/mono） |
| `Synthesizer` | class | 実装済 | 中核クラス。PhonemeEntry列を受け取り音声合成 |
| `CType` | enum | 実装済 | 子音種別（26種: V,K,G,T,D,P,B,CH,TS,DZH,DZ,S,SH,HA,HI,HU,HE,HO,NN,M,R,J,W,Q,HN,SK） |
| `KanaEntry` | struct | 実装済 | CType + 母音インデックスのペア（83エントリテーブル） |
| `emitCV` | 関数 | 実装済 | CType→PhonemeEntry列の生成 |
| `textToPhoneme` | 関数 | 実装済 | ひらがなUTF-8 → 音素列変換（拗音・促音・撥音・長音対応） |

設計原則: 継承なし、仮想関数なし、`enum class` + switch で分岐。

### 音源モデル（3種類）

| 音源 | 用途 | 実装 |
|------|------|------|
| インパルス列 | 母音、鼻音、半母音、弾き音、有声破裂音のvoice bar | 周期 = fs/f0 ごとにパルス |
| 白色雑音 | 無声摩擦音、破裂バースト、気息(VOT区間) | 線形合同法で [-1,+1] |
| 混合 | 有声摩擦音 /z, ʑ/ | インパルス + ノイズ加算 |

### 2次IIR共振器の係数計算
```
R  = exp(-PI * bw / fs)
b1 = -2.0 * R * cos(2.0 * PI * f / fs)
b2 = R * R
a0 = 1.0 + b1 + b2    // 簡易ゲイン補正
y  = a0*x - b1*z1 - b2*z2
```

### 母音フォルマントパラメータ（F1, F2, F3 [Hz]）
| 母音 | F1   | F2   | F3   | BW1 | BW2 | BW3 |
|------|------|------|------|-----|-----|-----|
| あ   | 800  | 1200 | 2600 | 80  | 100 | 120 |
| い   | 300  | 2300 | 3000 | 80  | 120 | 150 |
| う   | 350  | 1300 | 2500 | 80  | 100 | 120 |
| え   | 500  | 1900 | 2600 | 80  | 100 | 120 |
| お   | 500  | 800  | 2400 | 80  | 100 | 120 |

### フォルマントロカス（CV遷移の開始値）

| 調音点 | 該当子音 | F2ロカス(Hz) | F3ロカス(Hz) |
|--------|---------|-------------|-------------|
| 両唇 | /p,b,m,ɸ,w/ | 800-1000 | 2200-2400 |
| 歯茎 | /t,d,n,s,z,ɾ,ts,dz/ | 1700-1800 | 2600-2900 |
| 歯茎硬口蓋 | /ɕ,ʑ,tɕ,dʑ/ | 2000-2400 | 2800-3200 |
| 硬口蓋 | /ç,j/ | 2100-2500 | 2800-3200 |
| 軟口蓋 | /k,g/ | 母音依存 | 母音依存 |
| 声門 | /h/ | 母音値に近い | 母音値に近い |

### テキスト入力

ひらがな/カタカナ限定（UTF-8、各文字3バイト、U+3041〜U+3093）。漢字は非対応。

## Milestones

| MS | 内容 | 累計行数 | リスク | 状態 |
|----|------|---------|--------|------|
| 1 | パイプライン骨格 — 「あ」が出る | ~145行 | 中 | ✅ 完了 |
| 2 | 5母音 + フォルマント遷移 | ~215行 | 低 | ✅ 完了 |
| 3 | ノイズ音源 + 7子音 /h,s,n,m,ɾ,j,w/ | ~262行 | 中 | ✅ 完了 |
| R1 | Synthesizerクラス抽出 + lerp関数 | ~276行 | — | ✅ 完了 |
| 4 | 破裂音 /p,b,t,d,k,g/ + 促音 + 撥音 | ~360行 | ★最高 | ✅ 完了 |
| 5 | 破擦音 + 残り摩擦音 → 全音素完成 | ~399行 | 中高 | ✅ 完了 |
| 6 | ひらがなテキスト入力対応 | ~437行 | 低 | ✅ 完了 |
| 7 | （任意）ピッチ制御 + 品質改善 | ~628行 | — | |

リファクタリングポイント: ~~MS2後~~R1完了、MS4後、MS6後。
テスト: 19スイート / 176ケース（`make test`で実行）

## Reference

- 基本パラメータ: サンプルレート 44100Hz, 基本周波数 160Hz, 振幅 0.9 * 32767
- 参考実装: eSpeak NG `klatt.c`（~820行、C言語）
- 文献: Klatt (1980) "Software for a cascade/parallel formant synthesizer" JASA 67(3); Stevens (1998) *Acoustic Phonetics*
