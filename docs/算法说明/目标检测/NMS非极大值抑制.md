# 目标检测 / NMS 非极大值抑制

## 功能说明

非极大值抑制（Non-Maximum Suppression, NMS）是目标检测后处理的核心步骤，用于消除同一目标上重复的检测框，保留最优结果。

## 输入与输出

- 输入：候选检测框列表（含类别、置信度、边界框）+ IoU 阈值
- 输出：去重后的检测框列表

## 参数

| 参数 | 类型 | 必填 | 默认值 | 说明 |
| --- | --- | --- | --- | --- |
| nms_threshold | float | 是 | `0.45` | IoU 阈值，高于此值的同类别重叠框将被抑制 |

## 算法内容

### IoU 计算

交并比（Intersection over Union）衡量两个边界框的重叠程度：

```
IoU = Area(A ∩ B) / Area(A ∪ B)
```

其中：
- `Area(A ∩ B)` 为两个框交集区域的面积
- `Area(A ∪ B)` 为两个框并集区域的面积
- IoU 范围为 `[0, 1]`，0 表示无重叠，1 表示完全重合

### NMS 算法步骤

1. **按置信度排序**：将所有候选框按置信度从高到低排序。
2. **贪心选择**：依次取出置信度最高的候选框加入结果集。
3. **抑制重叠框**：对每个已选框，计算其与剩余候选框的 IoU；若 IoU > `nms_threshold`，则抑制该候选框。
4. **迭代**：重复步骤 2–3，直到所有候选框被处理或抑制。

### 按类别分组

本实现按类别 ID 分组执行 NMS，即只对同一类别的检测框进行抑制。不同类别的检测框即使高度重叠也不会互相抑制。

伪代码：

```
for each class_id:
    candidates = all_detections.filter(class_id)
    sorted = candidates.sort_by(confidence, descending)

    kept = []
    for candidate in sorted:
        suppressed = false
        for kept_box in kept:
            if IoU(candidate.box, kept_box.box) > nms_threshold:
                suppressed = true
                break
        if not suppressed:
            kept.append(candidate)

    results.extend(kept)

results.sort_by(confidence, descending)
```

## 处理流程

1. 置信度过滤后的候选检测按 `classId` 分组。
2. 每组内按置信度降序排列。
3. 对每组执行贪心 NMS。
4. 合并所有组的保留结果，按置信度降序输出。

## 输出说明

- 输出为去重后的 `DetectionItem` 列表
- 同一目标最多保留一个检测框（置信度最高的那个）
- 不同类别的检测框互不影响

## 结果解释

- `nms_threshold` 越低，抑制越激进，重叠框越少
- `nms_threshold` 越高，保留越多，可能出现同一目标的多个检测框
- 典型值：YOLOv8 默认 `0.45`

## 阈值选择建议

| 场景 | 推荐阈值 | 说明 |
| --- | --- | --- |
| 通用目标检测 | 0.45 | YOLOv8 默认值，平衡精度与召回 |
| 密集目标 | 0.3–0.4 | 更激进去重，减少冗余框 |
| 稀疏目标 | 0.5–0.6 | 保留更多候选，避免漏检 |

## 适用场景

- YOLO 系列模型的后处理去重
- 任何产生重叠检测框的目标检测模型

## 注意事项

- NMS 仅在同类别检测框之间执行，不同类别不会互相抑制
- IoU 计算基于边界框的交集与并集面积比
- 当两个框完全不重叠时 IoU = 0，不会被抑制
- 本实现为贪心 NMS（Greedy NMS），时间复杂度 O(N²)
