# C++ アーキテクチャ設計・マイルストーン

5エージェントによる並列調査の総合結果。

---

## 1. C++ 設計方針

### 言語: C++20

```bash
clang++ -std=c++20 -O2 -Wall -o formant formant.cpp
./formant "こんにちは"    # output.wav を生成
afplay output.wav         # macOS で再生
```

macOS の clang++ は C++20 を完全サポート。Linux では `g++ -std=c++20` で同等。
`-lm` は C++ では通常不要だが付けても無害。

### ビルドシステム: Makefile（10行程度）

```makefile
CXX       ?= c++
CXXFLAGS   = -std=c++20 -O2 -Wall -Wextra
TARGET     = formant

$(TARGET): formant.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) *.wav
```

CMake は 500行規模では過剰。単一ファイルコンパイルで十分。

### ファイル構成: 単一ファイル（~500行）

```
formant.cpp    // 全部入り1ファイル
Makefile       // ビルド用
```

ファイル内の論理的区切りをコメントで明示:
```cpp
// ============ Constants & Types ============
// ============ Resonator ============
// ============ Phoneme Data ============
// ============ Source Generation ============
// ============ Synthesizer ============
// ============ Text to Phoneme ============
// ============ WAV Writer ============
// ============ Main ============
```

---

## 2. クラス設計

「データ指向 + 薄いクラス」戦略。GoFパターン不要。継承・仮想関数なし。

### クラス/構造体一覧

| 名前 | 種別 | 行数 | 役割 |
|------|------|------|------|
| `Resonator` | struct + メソッド | ~35行 | 2次IIR共振器。状態(z1,z2)+係数+process() |
| `FormantParams` | struct (POD) | ~10行 | F1-F3, BW1-BW3, 振幅, 音源タイプ等 |
| `PhonemeTable` | constexpr配列 | ~80行 | 28-29音素のパラメータテーブル |
| `Synthesizer` | class | ~150行 | 中核。Resonator×3, 音源, 遷移, フェーズ制御 |
| `TextToPhoneme` | 関数群 | ~70行 | ひらがなUTF-8→音素列変換 |
| `WavWriter` | 関数1つ | ~30行 | WAVヘッダ+データ書き出し |

### 依存関係（一方向の木構造）

```
main()
  ├── TextToPhoneme::convert("こんにちは") -> vector<Phoneme>
  ├── Synthesizer
  │     ├── PhonemeTable (constexpr data)
  │     │     └── FormantParams (POD struct)
  │     ├── Resonator x3
  │     └── SourceType (enum class)
  └── WavWriter::write("output.wav", samples)
```

### Resonator（参考動画の BiquadFilter に相当）

```cpp
struct Resonator {
    double z1 = 0, z2 = 0;
    double a0, b1, b2;

    void set(double freq, double bw, double fs);
    double process(double input);
    void reset() { z1 = z2 = 0; }
};
```

- 参考動画の `setParam`/`filter` と同等
- コンストラクタで `z1=z2=0` を保証（元コードの改善点）
- 反共振器は `antiresonator()` として別メソッドまたは別関数で追加可能

### Synthesizer（中核クラス）

```cpp
class Synthesizer {
public:
    void synthesize(const std::vector<FormantParams>& phonemes,
                    std::vector<int16_t>& output);
private:
    std::array<Resonator, 3> filters_;
    double phase_ = 0;          // 音源の位相カウンタ
    uint32_t noise_seed_;       // ノイズ生成用シード

    double generateSource(SourceType type, double f0, double fs);
    void updateFormants(const FormantParams& target, double fs);
};
```

- eSpeak NG の `parwave()` + `frame_init()` に相当
- フィルタ係数はフレーム単位（5ms=220サンプル）で更新
- 振幅はサンプル単位で線形補間

---

## 3. C++20 機能の活用

### 使う機能

| 機能 | 用途 |
|------|------|
| `enum class` | `Phoneme`, `SourceType`, `Phase` の型安全な列挙 |
| `std::array<T,N>` | Resonator×3段、フォルマントテーブル |
| `std::vector<int16_t>` | 合成バッファ（可変長） |
| `constexpr` | フォルマントパラメータテーブル、数学定数 |
| `std::string` / `std::string_view` | テキスト入力のUTF-8処理 |
| `<cstdint>` | WAV出力の固定サイズ整数型 |
| `std::numbers::pi` | M_PI の代替として数学定数に使用 |
| designated initializers | `FormantParams{.f1=800, .f2=1200, ...}` で可読性向上 |
| `std::span` | vector参照の代わりに使える（任意） |

### 使わない機能

- `std::unique_ptr` / `std::shared_ptr`（ヒープ確保なし）
- `std::thread`（バッチ合成なのでシングルスレッド）
- RTTI / `dynamic_cast`（多態なし）
- 例外（`-fno-exceptions` でビルド可能に）
- テンプレートメタプログラミング
- `std::variant` / `std::any`

---

## 4. eSpeak NG klatt.c から学ぶ設計原則

eSpeak NG の `klatt.c` は約820行（コアエンジン約490行）。以下の3つの設計判断を本プロジェクトでも採用する:

### 原則1: 共振器を最小単位として設計

`resonator_t` + `resonator()` + `setresonator()` の約30行が全フィルタ処理の心臓。
→ 本プロジェクトの `Resonator` struct がこれに対応。

### 原則2: カスケードとパラレルを振幅パラメータで制御

if/else 切り替えではなく、AV=0 なら自然にカスケードが沈黙、AF=0 ならパラレルが沈黙。
→ 有声摩擦音のような中間的な音が追加コードなしで実現可能。

### 原則3: フィルタ係数はフレーム単位、振幅はサンプル単位

三角関数を含む係数計算はフレーム（5-10ms）ごと、振幅の線形補間はサンプルごと。
→ 計算コストと音質のバランスの鍵。

---

## 5. テスト方法

### 基本サイクル

```bash
make && ./formant "あいうえお" && afplay output.wav
```

3秒以内で完了するなら、テストフレームワーク不要。

### 検証手段

| 方法 | ツール | 目的 |
|------|--------|------|
| 聴覚確認 | `afplay output.wav` | 一次判定 |
| スペクトログラム | Audacity | フォルマント位置の視覚確認（最重要） |
| 波形表示 | Audacity / `sox` | クリッピング・不連続の検出 |

### テストフレームワーク

500行規模では **不要**。`assert()` で十分。

---

## 6. マイルストーン

### 全体像

```
MS1 → MS2 → [refactor] → MS3 → MS4 → [refactor] → MS5 → MS6 → [refactor] → MS7(任意)
```

### MS1: パイプライン骨格 — 「あ」が出る（~145行）

| タスク | 行数 |
|--------|------|
| WAVヘッダ書き出し関数 | ~30行 |
| インパルス列音源（f0=160Hz） | ~15行 |
| Resonator struct（set + process） | ~35行 |
| 3段直列フィルタで母音「あ」合成 | ~20行 |
| サンプル→WAV書き出し | ~25行 |
| main関数 | ~20行 |

**完了条件**: `afplay output.wav` で「あーー」と聞こえる。スペクトログラムで F1=800, F2=1200, F3=2600Hz にピーク確認。

**技術リスク: 中** — IIR係数の計算ミスで発振する危険。

### MS2: 5母音 + フォルマント遷移（累計 ~215行）

| タスク | 行数 |
|--------|------|
| 母音フォルマントテーブル（5母音） | ~15行 |
| 音素列データ構造 | ~10行 |
| フレーム単位の線形補間エンジン（5ms=220サンプル） | ~30行 |
| 母音シーケンス合成ループ | ~15行 |

**完了条件**: 「あいうえお」が滑らかに繋がって聞こえる。遷移でクリックなし。

**技術リスク: 低** — MS1の構造を拡張するだけ。

**リファクタリングポイント1**: Resonator/WAV出力のインターフェース確定。

### MS3: ノイズ音源 + 7子音（累計 ~315行）

| タスク | 行数 |
|--------|------|
| ノイズ音源生成（白色雑音） | ~10行 |
| 音源タイプ列挙と切り替え | ~15行 |
| 子音パラメータ（/h,s,n,m,r,j,w/） | ~25行 |
| 半母音 /j,w/（遷移のみ） | ~10行 |
| 弾き音 /ɾ/（短パルス） | ~10行 |
| 鼻音 /m,n/（F1≈250Hz, 広帯域） | ~15行 |
| 摩擦音 /h,s/（ノイズ+BPF） | ~15行 |

**完了条件**: 「はさなまらやわ」行が聞き取れる。

**技術リスク: 中** — ノイズと声帯音源の振幅バランス調整。

### MS4: 破裂音 + 促音 + 撥音（累計 ~405行）★最高リスク★

| タスク | 行数 |
|--------|------|
| 破裂音フェーズ制御ステートマシン | ~30行 |
| 無声破裂音 /p,t,k/ パラメータ | ~15行 |
| 有声破裂音 /b,d,g/ パラメータ（voice bar含む） | ~15行 |
| フォルマントロカステーブル（調音点×母音） | ~15行 |
| 促音 /Q/（無音区間120ms） | ~5行 |
| 撥音 /N/（簡易版：一律[n]で近似） | ~10行 |

**完了条件**: 「かたぱ」が区別でき、「がだば」が有声に聞こえる。「かっぱ」で促音が感じられる。

**技術リスク: ★最高★** — VOTの時間配分、軟口蓋音のvelar pinch（後続母音依存）。

**リファクタリングポイント2**: フェーズ制御・テーブル構造の整理。

### MS5: 破擦音 + 残り摩擦音 → 全音素完成（累計 ~473行）

| タスク | 行数 |
|--------|------|
| 破擦音（破裂+長摩擦の複合制御） | ~20行 |
| 条件異音パラメータ（し,ち,つ,ひ,ふ） | ~15行 |
| 有声摩擦音の混合音源 /z,ʑ/ | ~10行 |
| 全音素パラメータテーブル完成 | ~20行 |
| 四つ仮名統合（ぢ=じ、づ=ず） | ~3行 |

**完了条件**: 五十音表のすべて（46清音+20濁音+5半濁音）が聞き取れる。

**技術リスク: 中高** — 破擦音の破裂→摩擦接続タイミング。

### MS6: ひらがなテキスト入力（累計 ~558行）

| タスク | 行数 |
|--------|------|
| UTF-8→コードポイント変換 | ~15行 |
| ひらがな→音素マッピングテーブル（71仮名） | ~35行 |
| 拗音の先読み処理 | ~15行 |
| 促音・撥音・長音の特殊処理 | ~10行 |
| main改修（コマンドライン入力） | ~10行 |

**完了条件**: `./formant "こんにちは"` で「こんにちは」と聞こえる。

**技術リスク: 低** — テーブル変換のみ、音声処理に変更なし。

**リファクタリングポイント3**: 全体整理。

### MS7（任意）: ピッチ制御 + 品質改善（累計 ~628行）

| タスク | 行数 |
|--------|------|
| ピッチ制御（モーラ単位H/L, f0線形補間） | ~25行 |
| 振幅エンベロープ（Attack/Release） | ~15行 |
| 撥音の異音精緻化（後続音依存） | ~10行 |
| 反共振フィルタ（鼻音改善） | ~20行 |

---

## 7. 技術リスクランキング

| 順位 | リスク | 該当MS | 対策 |
|------|--------|--------|------|
| 1 | 破裂音のフェーズ制御 | MS4 | eSpeak NGのVOTパラメータ参照、個別チューニング |
| 2 | IIR共振器の安定性 | MS1 | double精度、R<1の検証 |
| 3 | ノイズ/声帯音源の振幅バランス | MS3 | 個別ゲイン係数で調整可能に |
| 4 | 軟口蓋音のvelar pinch | MS4 | 後続母音F2で条件分岐 |
| 5 | 破擦音の破裂→摩擦接続 | MS5 | フェーズ間の振幅クロスフェード |

---

## 8. 参考文献・実装

- **eSpeak NG** `klatt.c` (~820行, C): https://github.com/espeak-ng/espeak-ng
- **Klatt (1980)** "Software for a cascade/parallel formant synthesizer" JASA 67(3)
- **Stevens (1998)** *Acoustic Phonetics*, MIT Press
