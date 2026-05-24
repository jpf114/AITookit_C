"""
AIToolkit_C 内置模型推理性能基准

用法:
  python scripts/benchmark_models.py
  python scripts/benchmark_models.py --warmup 3 --iterations 20
  python scripts/benchmark_models.py --model yolov8n

输出每个模型的预热后多次推理延迟（mean / p50 / p95）与估算 FPS。
"""

from __future__ import annotations

import argparse
import statistics
import sys
import time
from pathlib import Path

import cv2
import numpy as np

try:
    import onnxruntime as ort
except ImportError:
    print("错误: 需要安装 onnxruntime: pip install onnxruntime")
    sys.exit(1)

# 复用 E2E 脚本的预处理与标签解析
SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR))

from test_builtin_models import (  # noqa: E402
    MODELS_DIR,
    create_test_image,
    load_manifest,
    preprocess_classification,
    preprocess_detection,
    resolve_manifest_labels,
)


def percentile(values: list[float], pct: float) -> float:
    if not values:
        return 0.0
    ordered = sorted(values)
    index = min(len(ordered) - 1, max(0, int(round((pct / 100.0) * (len(ordered) - 1)))))
    return ordered[index]


def benchmark_manifest(
    manifest_path: Path,
    test_image: np.ndarray,
    warmup: int,
    iterations: int,
) -> dict | None:
    name = manifest_path.stem
    try:
        manifest = load_manifest(manifest_path)
    except Exception as error:
        print(f"  [SKIP] {name}: manifest 解析失败 - {error}")
        return None

    if "model" not in manifest:
        print(f"  [SKIP] {name}: manifest 缺少 model 字段")
        return None

    model_path = MODELS_DIR / manifest["model"]
    if not model_path.is_file():
        print(f"  [SKIP] {name}: ONNX 不存在 ({manifest['model']})")
        return None

    labels = resolve_manifest_labels(manifest, manifest_path)
    if not labels and manifest.get("task_type") in ("detection", "segmentation", "classification"):
        print(f"  [SKIP] {name}: 无法解析标签")
        return None

    session = ort.InferenceSession(str(model_path), providers=["CPUExecutionProvider"])
    input_name = session.get_inputs()[0].name
    input_w = int(manifest["input_width"])
    input_h = int(manifest["input_height"])
    task_type = manifest["task_type"]

    if task_type == "classification":
        resized = cv2.resize(test_image, (input_w, input_h))
        blob = preprocess_classification(resized, input_w, input_h)
    else:
        blob = preprocess_detection(test_image, input_w, input_h)

    for _ in range(warmup):
        session.run(None, {input_name: blob})

    samples_ms: list[float] = []
    for _ in range(iterations):
        start = time.perf_counter()
        session.run(None, {input_name: blob})
        samples_ms.append((time.perf_counter() - start) * 1000.0)

    mean_ms = statistics.mean(samples_ms)
    return {
        "name": name,
        "task": task_type,
        "input": f"{input_w}x{input_h}",
        "mean_ms": mean_ms,
        "p50_ms": percentile(samples_ms, 50),
        "p95_ms": percentile(samples_ms, 95),
        "fps": 1000.0 / mean_ms if mean_ms > 0 else 0.0,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="AIToolkit_C 模型推理性能基准")
    parser.add_argument("--models-dir", type=Path, default=MODELS_DIR, help="模型目录")
    parser.add_argument("--model", type=str, default="", help="仅测试指定 manifest 名称（不含扩展名）")
    parser.add_argument("--warmup", type=int, default=5, help="预热次数")
    parser.add_argument("--iterations", type=int, default=30, help="计时迭代次数")
    parser.add_argument("--image", type=str, default="", help="测试图片（默认生成 640x480）")
    args = parser.parse_args()

    models_dir = args.models_dir.resolve()
    if not models_dir.is_dir():
        print(f"错误: 模型目录不存在 {models_dir}")
        sys.exit(1)

    if args.image:
        test_image = cv2.imread(args.image)
        if test_image is None:
            print(f"错误: 无法读取图片 {args.image}")
            sys.exit(1)
    else:
        test_image = create_test_image(640, 480)

    manifests = sorted(models_dir.glob("*.json"))
    if args.model:
        manifests = [p for p in manifests if p.stem == args.model]
        if not manifests:
            print(f"错误: 未找到 manifest {args.model}.json")
            sys.exit(1)

    print("=" * 78)
    print("  AIToolkit_C 模型推理性能基准")
    print("=" * 78)
    print(f"  目录: {models_dir}")
    print(f"  预热: {args.warmup}  迭代: {args.iterations}  Provider: CPUExecutionProvider")
    print()

    results: list[dict] = []
    for manifest_path in manifests:
        row = benchmark_manifest(manifest_path, test_image, args.warmup, args.iterations)
        if row:
            results.append(row)

    if not results:
        print("没有可基准测试的模型。")
        sys.exit(1)

    print(f"{'模型':<18} {'任务':<14} {'输入':<10} {'mean':>8} {'p50':>8} {'p95':>8} {'FPS':>8}")
    print("-" * 78)
    for row in results:
        print(
            f"{row['name']:<18} {row['task']:<14} {row['input']:<10} "
            f"{row['mean_ms']:>7.1f}ms {row['p50_ms']:>7.1f}ms {row['p95_ms']:>7.1f}ms {row['fps']:>7.1f}"
        )
    print("=" * 78)


if __name__ == "__main__":
    main()
