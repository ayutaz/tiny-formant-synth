# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

C++17によるフォルマント音声合成器。日本語の全音素（母音・子音・濁音・半濁音・拗音・促音・撥音）を合成し、WAVファイルとして出力する。最終目標は約500行のコンパクトな単一ファイル実装。

## Build & Run

```bash
# 直接コンパイル
clang++ -std=c++17 -O2 -Wall -o formant formant.cpp

# または Makefile
make

# 実行・再生
./formant "こんにちは"    # output.wav を生成
afplay output.wav         # macOS で再生
```

## Architecture

### 合成方式（Klatt簡易版）

```
音源（インパルス列 or ノイズ or 混合）
  → 2次IIR共振器（Resonator）3段直列（F1/F2/F3）
  → WAV出力（16bit/44100Hz/mono）
```

### クラス設計（データ指向 + 薄いクラス）

| 名前 | 種別 | 役割 |
|------|------|------|
| `Resonator` | struct | 2次IIR共振器。z1,z2 + set() + process() |
| `FormantParams` | struct (POD) | F1-F3, BW1-BW3, 振幅, 音源タイプ |
| `PhonemeTable` | constexpr配列 | 28-29音素のFormantParamsテーブル |
| `Synthesizer` | class | 中核。Resonator×3, 音源, フォルマント遷移, フェーズ制御 |
| `TextToPhoneme` | 関数群 | ひらがなUTF-8 → 音素列変換 |
| `WavWriter` | 関数 | WAVヘッダ+データ書き出し |

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

| MS | 内容 | 累計行数 | リスク |
|----|------|---------|--------|
| 1 | パイプライン骨格 — 「あ」が出る | ~145行 | 中 |
| 2 | 5母音 + フォルマント遷移 | ~215行 | 低 |
| 3 | ノイズ音源 + 7子音 /h,s,n,m,r,j,w/ | ~315行 | 中 |
| 4 | 破裂音 /p,b,t,d,k,g/ + 促音 + 撥音 | ~405行 | ★最高 |
| 5 | 破擦音 + 残り摩擦音 → 全音素完成 | ~473行 | 中高 |
| 6 | ひらがなテキスト入力対応 | ~558行 | 低 |
| 7 | （任意）ピッチ制御 + 品質改善 | ~628行 | — |

リファクタリングポイント: MS2後、MS4後、MS6後。

## Reference

- 参考動画: `ssstwitter.com_1772771421555.mp4`（元のC++実装のコード画面）
- 全音素調査: `docs/research-full-phoneme-synthesis.md`
- C++設計・マイルストーン詳細: `docs/cpp-architecture.md`
- 基本パラメータ: サンプルレート 44100Hz, 基本周波数 160Hz, 振幅 0.9 * 32767
- 参考実装: eSpeak NG `klatt.c`（~820行、C言語）
- 文献: Klatt (1980) "Software for a cascade/parallel formant synthesizer" JASA 67(3); Stevens (1998) *Acoustic Phonetics*
