# tiny-formant-synth

**437行のC++で日本語を喋る -- A tiny Japanese formant speech synthesizer in ~437 lines of C++**

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

Klatt簡易モデルに基づくフォルマント音声合成器。外部ライブラリに一切依存せず、単一のC++ファイル（`formant.cpp`）だけで日本語の全音素を合成しWAVファイルとして出力します。

> A minimal formant speech synthesizer for Japanese, based on a simplified Klatt model.
> Single-file C++ implementation with zero external dependencies.

## Features

- **単一ファイル実装** -- `formant.cpp` のみ（約437行）、外部ライブラリ不要
- **全日本語音素対応** -- 5母音、破裂音、破擦音、摩擦音、鼻音、弾き音、半母音、促音、撥音
- **ひらがなテキスト入力** -- コマンドライン引数でひらがなを渡すだけ
- **Klatt簡易モデル** -- インパルス列 / ノイズ / 混合音源 → 2次IIR共振器 x3段カスケード
- **16bit PCM WAV出力** -- 44100Hz, mono

## Requirements

- **C++20** 対応コンパイラ
  - clang++ 13+ (macOS Xcode 13+)
  - g++ 11+
- **OS**: macOS / Linux
- **Make** (任意 -- 直接コンパイルも可)

## Build & Run

```bash
# Makefile を使う場合
make
./formant "こんにちは"

# 直接コンパイル
clang++ -std=c++20 -O2 -Wall -o formant formant.cpp
./formant "さくら"
```

## Usage

```bash
# ひらがなテキストを引数に渡す
./formant "あいうえお"
# => Wrote output.wav (26460 samples, 5 segments)

# デフォルト（引数なし）は "あいうえお"
./formant

# 拗音・促音・撥音にも対応
./formant "きょうはいいてんきです"
./formant "にっぽん"
./formant "しんぶん"

# macOS で再生
afplay output.wav

# Linux で再生（aplay がある場合）
aplay output.wav
```

**注意**: 入力はひらがな（とカタカナ長音「ー」）のみ対応です。漢字・英数字は無視されます。

## Architecture

```
テキスト入力（ひらがな UTF-8）
  │
  ▼
textToPhoneme()  ── ひらがな→音素列変換（kKanaTable + 拗音/促音/撥音処理）
  │
  ▼
PhonemeEntry列   ── [{FormantParams, duration_samples}, ...]
  │
  ▼
Synthesizer::synthesize()
  │
  ├─ 音源生成 ─── ImpulseTrain（有声音）/ NoiseGen（無声音）/ Mixed（有声摩擦音）
  │
  ├─ フィルタ ─── Resonator x3 直列（F1, F2, F3）── 2次IIR共振器
  │               └ 5msフレームごとに係数更新、音素間で線形補間
  │
  ▼
writeWav()       ── 16bit PCM WAV (44100Hz, mono) 出力
```

### 合成方式（Klatt簡易版）

- **音源**: 3種類（インパルス列 / 白色雑音 / 混合）を `SourceType` で切替
- **フィルタ**: 2次IIR共振器（`Resonator`）をF1/F2/F3の3段カスケード接続
- **遷移**: 各音素の末尾30%区間で次の音素へ線形補間（フォルマントロカス）
- **フレーム**: 5ms（220サンプル）ごとにフィルタ係数を再計算

## Supported Phonemes

| 種別 | 音素 |
|------|------|
| 母音 | あ い う え お |
| 無声摩擦音 | /s/、/ɕ/(し)、/ç/(ひ)、/ɸ/(ふ)、/h/ |
| 有声摩擦音 | /z/、/ʑ/(じ) |
| 無声破擦音 | /tɕ/(ち)、/ts/(つ) |
| 有声破擦音 | /dʑ/(ぢ/じ)、/dz/(づ/ず) |
| 鼻音 | /n/(な行)、/m/(ま行) |
| 弾き音 | /ɾ/(ら行) |
| 半母音 | /j/(や行)、/w/(わ) |
| 破裂音 | /p/、/b/、/t/、/d/、/k/、/g/ |
| 促音 | っ（120ms無音） |
| 撥音 | ん（80ms鼻音） |

## Samples

[docs/samples/](docs/samples/) にサンプル音声・動画があります。

| ファイル | 内容 |
|----------|------|
| [all.mp4](docs/samples/all.mp4) | 5文章の結合動画（テキスト表示付き） |
| [こんにちは.wav](docs/samples/こんにちは.wav) | こんにちは |
| [おはようございます.wav](docs/samples/おはようございます.wav) | おはようございます |
| [ありがとう.wav](docs/samples/ありがとう.wav) | ありがとう |
| [さようなら.wav](docs/samples/さようなら.wav) | さようなら |
| [はじめまして.wav](docs/samples/はじめまして.wav) | はじめまして |

## Testing

```bash
make test    # 全19スイート / 176テストケースを実行
```

外部依存なしの軽量テストフレームワーク（`test_framework.h`）を使用。

<details>
<summary>テストスイート一覧（19スイート）</summary>

| スイート | 対象 | ケース数 |
|---------|------|---------|
| Resonator | 2次IIR共振器 | 5 |
| NoiseGen | LCG乱数生成 | 5 |
| ImpulseTrain | インパルス列 | 5 |
| Lerp | FormantParams線形補間 | 6 |
| Helpers | ms2s / append / makePlosiveCV | 10 |
| PhonemeData | 音素定数値 | 22 |
| SynthBasic | 合成器基本動作 | 6 |
| SynthSource | 音源切替・遷移 | 8 |
| WAVWriter | WAVファイル出力 | 9 |
| Integration | フルパイプライン | 12 |
| MixedSource | 混合音源 | 6 |
| NewFricatives | 新摩擦音定数 | 10 |
| Affricates | 破擦音生成 | 10 |
| HaRow | は行条件異音 | 8 |
| SaRow | さ行 | 8 |
| ZaRow | ざ行 | 10 |
| TextToPhoneme | ひらがな→音素変換 | 14 |
| YouonSpecial | 拗音・促音・撥音・長音 | 12 |
| IntegrationMS6 | テキスト→合成パイプライン | 10 |

</details>

## References

- Klatt, D.H. (1980) "Software for a cascade/parallel formant synthesizer" *JASA* 67(3)
- Stevens, K.N. (1998) *Acoustic Phonetics*
- [eSpeak NG](https://github.com/espeak-ng/espeak-ng) -- klatt.c 参考実装

## Contributing

Issue や Pull Request を歓迎します。

- バグ報告: [Issues](../../issues) で日本語/英語どちらでもOK
- 機能提案・改善: PR を送る前に Issue で相談してください
- コードスタイル: 既存のコードに合わせてください（C++20、単一ファイル構成維持）

## License

[MIT](LICENSE)
