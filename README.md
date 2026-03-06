# tiny-formant-synth

C++20で書かれたコンパクトなフォルマント音声合成器。日本語の音声をWAVファイルとして生成。

**現在の状態**: MS4完了（破裂音・促音・撥音を含む合成が可能）

## 特徴

- 単一ファイル実装（`formant.cpp`、約362行）
- 外部ライブラリ依存なし（標準ライブラリのみ）
- Klatt簡易モデル（インパルス列/ノイズ音源 → 2次IIR共振器×3段カスケード）
- 16bit PCM WAV出力（44100Hz, mono）

## ビルド・実行

```bash
make
./formant
afplay output.wav    # macOS
make test            # テスト実行（10スイート / 67ケース）
```

## 動作原理

```
音源（インパルス列 or ノイズ）→ Resonator(2次IIR) ×3段直列 → WAV
```

- フレーム単位（5ms / 220サンプル）でフィルタ係数を更新
- 音素間で線形補間し、滑らかなフォルマント遷移を実現

## 対応音素

| 種別 | 音素 |
|------|------|
| 母音 | あ い う え お |
| 摩擦音 | は行 /h/、さ行 /s/ |
| 鼻音 | な行 /n/、ま行 /m/ |
| 弾き音 | ら行 /ɾ/ |
| 半母音 | や行 /j/、わ行 /w/ |
| 破裂音 | /p/、/b/、/t/、/d/、/k/、/g/ |
| 促音 | っ |
| 撥音 | ん |

## テスト

`make test` で全10スイート / 67テストケースを実行。外部依存なしの軽量テストフレームワーク。

| スイート | 対象 | ケース数 |
|---------|------|---------|
| Resonator | 2次IIR共振器 | 5 |
| NoiseGen | LCG乱数生成 | 5 |
| ImpulseTrain | インパルス列 | 5 |
| Lerp | FormantParams線形補間 | 6 |
| Helpers | ms2s / append / makePlosiveCV | 10 |
| PhonemeData | 音素定数値 | 13 |
| SynthBasic | 合成器基本動作 | 6 |
| SynthSource | 音源切替・遷移 | 5 |
| WAVWriter | WAVファイル出力 | 9 |
| Integration | フルパイプライン | 7 |

## ロードマップ

| MS | 内容 |
|----|------|
| ~~**4**~~ | ~~破裂音 /p,b,t,d,k,g/ + 促音 + 撥音~~ **完了** |
| **5** | 破擦音 + 残り摩擦音 → 全音素 |
| **6** | ひらがなテキスト入力 |
| **7** | ピッチ制御（任意） |

## 参考文献

- Klatt, D.H. (1980) "Software for a cascade/parallel formant synthesizer" *JASA* 67(3)
- Stevens, K.N. (1998) *Acoustic Phonetics*
- [eSpeak NG](https://github.com/espeak-ng/espeak-ng) klatt.c

## ライセンス

TBD
