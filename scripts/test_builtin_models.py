"""
AIToolkit_C 内置模型端到端验证脚本
用法: python scripts/test_builtin_models.py [--image 图片路径]

功能:
  1. 验证所有 JSON manifest 文件可正确解析
  2. 验证所有 ONNX 模型可正确加载和推理
  3. 对每个模型执行推理并输出结果摘要
  4. 可选: 使用指定图片进行推理可视化
"""

import json
import os
import sys
import time
import argparse
from pathlib import Path

import cv2
import numpy as np

try:
    import onnxruntime as ort
except ImportError:
    print("错误: 需要安装 onnxruntime: pip install onnxruntime")
    sys.exit(1)


MODELS_DIR = Path(__file__).resolve().parent.parent / "models"
LABELS_DIR = Path(__file__).resolve().parent.parent / "resources" / "labels"
DEFAULT_COCO_LABELS_PATH = LABELS_DIR / "coco80.txt"
DEFAULT_IMAGENET_LABELS_PATH = LABELS_DIR / "imagenet1000.txt"


def load_labels_file(path: Path) -> list[str]:
    with open(path, "r", encoding="utf-8") as f:
        return [line.strip() for line in f if line.strip()]


def resolve_manifest_labels(manifest: dict, manifest_path: Path) -> list[str]:
    if "labels_inline" in manifest:
        return manifest["labels_inline"]
    labels_ref = manifest.get("labels")
    if labels_ref:
        labels_path = (manifest_path.parent / labels_ref).resolve()
        if labels_path.is_file():
            return load_labels_file(labels_path)
    task_type = manifest.get("task_type", "")
    if task_type in ("detection", "segmentation") and DEFAULT_COCO_LABELS_PATH.is_file():
        return load_labels_file(DEFAULT_COCO_LABELS_PATH)
    if task_type == "classification" and DEFAULT_IMAGENET_LABELS_PATH.is_file():
        return load_labels_file(DEFAULT_IMAGENET_LABELS_PATH)
    return []


def load_manifest(json_path: Path) -> dict:
    with open(json_path, "r", encoding="utf-8") as f:
        return json.load(f)


def create_test_image(width: int, height: int) -> np.ndarray:
    img = np.zeros((height, width, 3), dtype=np.uint8)
    cv2.rectangle(img, (width // 4, height // 4), (3 * width // 4, 3 * height // 4), (0, 255, 0), 2)
    cv2.circle(img, (width // 2, height // 2), min(width, height) // 6, (0, 0, 255), -1)
    cv2.putText(img, "TEST", (width // 4, height // 6), cv2.FONT_HERSHEY_SIMPLEX, 1.0, (255, 255, 255), 2)
    return img


def preprocess_detection(img: np.ndarray, input_w: int, input_h: int) -> np.ndarray:
    blob = cv2.dnn.blobFromImage(img, 1.0 / 255.0, (input_w, input_h), swapRB=True, crop=False)
    return blob.astype(np.float32)


def preprocess_classification(img: np.ndarray, input_w: int, input_h: int) -> np.ndarray:
    resized = cv2.resize(img, (input_w, input_h))
    blob = resized.astype(np.float32) / 255.0
    blob = blob.transpose(2, 0, 1)
    blob = np.expand_dims(blob, axis=0)
    return blob


def postprocess_detection(output: np.ndarray, conf_thresh: float, nms_thresh: float, labels: list) -> list:
    if output.ndim == 3:
        if output.shape[1] == 4 + len(labels):
            output = output.transpose(0, 2, 1)
        predictions = output[0]
    else:
        predictions = output

    results = []
    for pred in predictions:
        if pred.shape[0] == 4 + len(labels):
            cx, cy, w, h = pred[:4]
            class_scores = pred[4:]
        elif pred.shape[0] == 5 + len(labels):
            cx, cy, w, h, obj = pred[:5]
            class_scores = pred[5:] * obj
        else:
            continue

        class_id = int(np.argmax(class_scores))
        confidence = float(class_scores[class_id])
        if confidence < conf_thresh:
            continue

        x1 = float(cx - w / 2)
        y1 = float(cy - h / 2)
        x2 = float(cx + w / 2)
        y2 = float(cy + h / 2)

        label = labels[class_id] if class_id < len(labels) else f"class_{class_id}"
        results.append({
            "label": label,
            "confidence": round(confidence, 4),
            "bbox": [round(x1, 1), round(y1, 1), round(x2, 1), round(y2, 1)],
        })

    return results


def postprocess_classification(output: np.ndarray, labels: list, top_k: int = 5) -> list:
    scores = output[0]
    if scores.ndim > 1:
        scores = scores.flatten()

    top_indices = np.argsort(scores)[::-1][:top_k]
    results = []
    for idx in top_indices:
        label = labels[idx] if idx < len(labels) else f"class_{idx}"
        results.append({
            "label": label,
            "confidence": round(float(scores[idx]), 4),
        })
    return results


def postprocess_segmentation(outputs: list, conf_thresh: float, labels: list) -> list:
    det_output = outputs[0]
    if det_output.ndim == 3:
        if det_output.shape[1] == 4 + len(labels) + 32:
            det_output = det_output.transpose(0, 2, 1)

    results = []
    for pred in det_output[0]:
        attr_count = pred.shape[0]
        if attr_count == 4 + len(labels) + 32:
            cx, cy, w, h = pred[:4]
            class_scores = pred[4:4 + len(labels)]
        elif attr_count == 5 + len(labels) + 32:
            cx, cy, w, h, obj = pred[:5]
            class_scores = pred[5:5 + len(labels)] * obj
        else:
            continue

        class_id = int(np.argmax(class_scores))
        confidence = float(class_scores[class_id])
        if confidence < conf_thresh:
            continue

        label = labels[class_id] if class_id < len(labels) else f"class_{class_id}"
        results.append({
            "label": label,
            "confidence": round(confidence, 4),
            "bbox": [round(float(cx - w / 2), 1), round(float(cy - h / 2), 1),
                     round(float(cx + w / 2), 1), round(float(cy + h / 2), 1)],
        })

    return results


def test_manifest(manifest_path: Path) -> bool:
    name = manifest_path.name
    try:
        m = load_manifest(manifest_path)
        required_fields = ["name", "task_type", "backend", "model", "input_width", "input_height"]
        missing = [f for f in required_fields if f not in m]
        if missing:
            print(f"  [FAIL] {name}: 缺少字段 {missing}")
            return False

        model_path = MODELS_DIR / m["model"]
        if not model_path.exists():
            print(f"  [FAIL] {name}: 模型文件不存在 {m['model']}")
            return False

        if "labels_inline" not in m and "labels" not in m:
            print(f"  [FAIL] {name}: 缺少标签定义")
            return False

        labels = resolve_manifest_labels(m, manifest_path)
        if not labels:
            print(f"  [FAIL] {name}: 无法解析标签")
            return False

        print(f"  [ OK ] {name}: {m['name']} | {m['task_type']} | {m['input_width']}x{m['input_height']} | {len(labels)} labels")
        return True
    except Exception as e:
        print(f"  [FAIL] {name}: 解析错误 - {e}")
        return False


def test_inference(manifest_path: Path, test_image: np.ndarray) -> bool:
    name = manifest_path.name
    try:
        m = load_manifest(manifest_path)
        model_path = MODELS_DIR / m["model"]
        labels = resolve_manifest_labels(m, manifest_path)
        if not labels:
            print(f"  [FAIL] {name}: 无法解析标签")
            return False

        session = ort.InferenceSession(str(model_path), providers=["CPUExecutionProvider"])
        input_name = session.get_inputs()[0].name
        input_w = m["input_width"]
        input_h = m["input_height"]
        conf_thresh = m.get("confidence_threshold", 0.25)
        nms_thresh = m.get("nms_threshold", 0.45)

        if m["task_type"] == "classification":
            blob = preprocess_classification(test_image, input_w, input_h)
        else:
            blob = preprocess_detection(test_image, input_w, input_h)

        start = time.perf_counter()
        outputs = session.run(None, {input_name: blob})
        elapsed = (time.perf_counter() - start) * 1000

        if m["task_type"] == "detection":
            results = postprocess_detection(outputs[0], conf_thresh, nms_thresh, labels)
            print(f"  [ OK ] {name}: 推理耗时 {elapsed:.1f}ms | 检测到 {len(results)} 个目标")
            for r in results[:5]:
                print(f"         - {r['label']}: {r['confidence']:.2%} @ {r['bbox']}")
            if len(results) > 5:
                print(f"         ... 还有 {len(results) - 5} 个目标")

        elif m["task_type"] == "classification":
            results = postprocess_classification(outputs[0], labels)
            print(f"  [ OK ] {name}: 推理耗时 {elapsed:.1f}ms | Top-5 分类结果:")
            for r in results:
                print(f"         - {r['label']}: {r['confidence']:.2%}")

        elif m["task_type"] == "segmentation":
            results = postprocess_segmentation(outputs, conf_thresh, labels)
            print(f"  [ OK ] {name}: 推理耗时 {elapsed:.1f}ms | 分割到 {len(results)} 个实例")
            for r in results[:5]:
                print(f"         - {r['label']}: {r['confidence']:.2%} @ {r['bbox']}")
            if len(results) > 5:
                print(f"         ... 还有 {len(results) - 5} 个实例")
        else:
            print(f"  [WARN] {name}: 未知任务类型 {m['task_type']}")
            return False

        return True

    except Exception as e:
        print(f"  [FAIL] {name}: 推理错误 - {e}")
        import traceback
        traceback.print_exc()
        return False


def test_cxx_manifest_loading() -> bool:
    exe_path = Path(__file__).resolve().parent.parent / "build" / "tests" / "Release" / "test_model_manifest.exe"
    if not exe_path.exists():
        print("  [SKIP] C++ manifest 测试可执行文件不存在，请先编译项目")
        return True
    import subprocess
    env = os.environ.copy()
    env["QT_QPA_PLATFORM"] = "minimal"
    result = subprocess.run([str(exe_path)], capture_output=True, text=True, env=env, timeout=30)
    if result.returncode == 0:
        print("  [ OK ] C++ test_model_manifest 全部通过")
        return True
    else:
        print(f"  [FAIL] C++ test_model_manifest 失败:\n{result.stdout}\n{result.stderr}")
        return False


def main():
    parser = argparse.ArgumentParser(description="AIToolkit_C 内置模型验证")
    parser.add_argument("--image", type=str, help="测试图片路径（可选，默认生成测试图）")
    args = parser.parse_args()

    print("=" * 70)
    print("  AIToolkit_C 内置模型端到端验证")
    print("=" * 70)
    print()

    if not MODELS_DIR.exists():
        print(f"错误: 模型目录不存在 {MODELS_DIR}")
        sys.exit(1)

    json_files = sorted(MODELS_DIR.glob("*.json"))
    onnx_files = sorted(MODELS_DIR.glob("*.onnx"))

    print(f"模型目录: {MODELS_DIR}")
    print(f"发现 {len(json_files)} 个 manifest, {len(onnx_files)} 个 ONNX 模型")
    print()

    # --- 阶段 1: 文件完整性检查 ---
    print("--- 阶段 1: 文件完整性检查 ---")
    for onnx_file in onnx_files:
        size_mb = onnx_file.stat().st_size / (1024 * 1024)
        print(f"  {onnx_file.name}: {size_mb:.1f} MB")
    print()

    # --- 阶段 2: Manifest 解析验证 ---
    print("--- 阶段 2: Manifest 解析验证 ---")
    manifest_ok = 0
    manifest_fail = 0
    for jf in json_files:
        if test_manifest(jf):
            manifest_ok += 1
        else:
            manifest_fail += 1
    print(f"\n  结果: {manifest_ok} 通过, {manifest_fail} 失败\n")

    # --- 阶段 3: ONNX 推理验证 ---
    print("--- 阶段 3: ONNX 推理验证 ---")

    if args.image:
        test_image = cv2.imread(args.image)
        if test_image is None:
            print(f"错误: 无法读取图片 {args.image}")
            sys.exit(1)
        print(f"  使用图片: {args.image} ({test_image.shape[1]}x{test_image.shape[0]})")
    else:
        test_image = create_test_image(640, 480)
        print("  使用生成的测试图片 (640x480)")

    print()
    inference_ok = 0
    inference_fail = 0
    for jf in json_files:
        if test_inference(jf, test_image):
            inference_ok += 1
        else:
            inference_fail += 1
    print(f"\n  结果: {inference_ok} 通过, {inference_fail} 失败\n")

    # --- 阶段 4: C++ 单元测试 ---
    print("--- 阶段 4: C++ 单元测试 ---")
    cxx_ok = test_cxx_manifest_loading()
    print()

    # --- 总结 ---
    print("=" * 70)
    total_ok = manifest_ok + inference_ok + (1 if cxx_ok else 0)
    total_fail = manifest_fail + inference_fail + (0 if cxx_ok else 1)
    print(f"  验证完成: {total_ok} 通过, {total_fail} 失败")
    print("=" * 70)

    if total_fail > 0:
        sys.exit(1)
    else:
        print("\n  所有内置模型验证通过！")
        print("  你也可以启动 AIToolkit_C 应用程序进行人工测试:")
        print(f"    {Path(__file__).resolve().parent.parent / 'build' / 'src' / 'Release' / 'ai_toolkit_c.exe'}")


if __name__ == "__main__":
    main()
