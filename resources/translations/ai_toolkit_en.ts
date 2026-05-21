<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="en" sourcelanguage="zh_CN">
<context>
    <name>aitoolkit::ui::AppController</name>
    <message>
        <location filename="../../src/ui/app_controller.cpp" line="18"/>
        <source>GPU 推理不可用，已自动回退到 CPU 模式。如需 GPU 加速，请确认已安装 CUDA Toolkit 并使用启用 CUDA 的构建。</source>
        <translation>GPU inference unavailable, automatically fell back to CPU mode. For GPU acceleration, please ensure CUDA Toolkit is installed and use a CUDA-enabled build.</translation>
    </message>
    <message>
        <location filename="../../src/ui/app_controller.cpp" line="191"/>
        <source>请先加载模型清单，再进行批量推理。</source>
        <translation>Please load a model manifest before running batch inference.</translation>
    </message>
    <message>
        <location filename="../../src/ui/app_controller.cpp" line="207"/>
        <source>所选文件夹中没有找到支持的图像文件。</source>
        <translation>No supported image files found in the selected folder.</translation>
    </message>
    <message>
        <location filename="../../src/ui/app_controller.cpp" line="241"/>
        <source>请先加载模型清单，再进行视频推理。</source>
        <translation>Please load a model manifest before running video inference.</translation>
    </message>
    <message>
        <location filename="../../src/ui/app_controller.cpp" line="270"/>
        <source>请先加载模型清单。</source>
        <translation>Please load a model manifest first.</translation>
    </message>
    <message>
        <location filename="../../src/ui/app_controller.cpp" line="321"/>
        <source>无法保存渲染图片至：%1</source>
        <translation>Unable to save rendered image to: %1</translation>
    </message>
</context>
<context>
    <name>aitoolkit::ui::HomePage</name>
    <message>
        <location filename="../../src/ui/pages/home_page.cpp" line="19"/>
        <source>AI 检测工具 v%1</source>
        <translation>AI Detection Tool v%1</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/home_page.cpp" line="23"/>
        <source>从左侧开始加载模型清单，选择待推理图像后即可执行一次目标检测，并在结果页查看叠加预览与明细。</source>
        <translation>Start by loading a model manifest from the left panel, select an image for inference, then run object detection and view the overlay preview and details on the results page.</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/home_page.cpp" line="33"/>
        <source>加载模型</source>
        <translation>Load Model</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/home_page.cpp" line="36"/>
        <source>选择图像</source>
        <translation>Select Image</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/home_page.cpp" line="39"/>
        <source>下载示例模型</source>
        <translation>Download Sample Model</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/home_page.cpp" line="41"/>
        <source>下载 YOLOv8n COCO 示例模型（约 6MB）</source>
        <translation>Download YOLOv8n COCO sample model (approx. 6MB)</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/home_page.cpp" line="43"/>
        <source>模型目录</source>
        <translation>Model Catalog</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/home_page.cpp" line="45"/>
        <source>浏览和下载更多模型</source>
        <translation>Browse and download more models</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/home_page.cpp" line="47"/>
        <source>快速体验</source>
        <translation>Quick Start</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/home_page.cpp" line="49"/>
        <source>使用示例图像执行一次目标检测</source>
        <translation>Run object detection with a sample image</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/home_page.cpp" line="70"/>
        <source>最近模型</source>
        <translation>Recent Models</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/home_page.cpp" line="86"/>
        <source>最近图像</source>
        <translation>Recent Images</translation>
    </message>
</context>
<context>
    <name>aitoolkit::ui::ImagePreviewWidget</name>
    <message>
        <location filename="../../src/ui/widgets/image_preview_widget.cpp" line="45"/>
        <source>暂无可预览结果</source>
        <translation>No preview results available</translation>
    </message>
    <message>
        <location filename="../../src/ui/widgets/image_preview_widget.cpp" line="125"/>
        <source>%1%  双击重置</source>
        <translation>%1%  Double-click to reset</translation>
    </message>
</context>
<context>
    <name>aitoolkit::ui::InferencePage</name>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="21"/>
        <location filename="../../src/ui/pages/inference_page.cpp" line="84"/>
        <source>请先加载模型清单。</source>
        <translation>Please load a model manifest first.</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="24"/>
        <source>模型已就绪，请选择一张待推理图像。</source>
        <translation>Model is ready. Please select an image for inference.</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="26"/>
        <source>模型和图像已就绪，可以开始检测。</source>
        <translation>Model and image are ready. You can start detection.</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="37"/>
        <source>推理</source>
        <translation>Inference</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="41"/>
        <source>选择一张图像，在预览旁完成检测操作。</source>
        <translation>Select an image and complete detection next to the preview.</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="49"/>
        <location filename="../../src/ui/pages/inference_page.cpp" line="144"/>
        <source>选择图像</source>
        <translation>Select Image</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="52"/>
        <source>选择文件夹</source>
        <translation>Select Folder</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="55"/>
        <location filename="../../src/ui/pages/inference_page.cpp" line="162"/>
        <source>选择视频</source>
        <translation>Select Video</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="58"/>
        <source>最大帧数：</source>
        <translation>Max Frames:</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="62"/>
        <source>全部</source>
        <translation>All</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="63"/>
        <source>设置为 0 表示处理视频的所有帧</source>
        <translation>Set to 0 to process all video frames</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="65"/>
        <source>开始检测</source>
        <translation>Start Detection</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="69"/>
        <source>取消</source>
        <translation>Cancel</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="80"/>
        <location filename="../../src/ui/pages/inference_page.cpp" line="180"/>
        <location filename="../../src/ui/pages/inference_page.cpp" line="208"/>
        <source>当前未选择图像</source>
        <translation>No image currently selected</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="103"/>
        <source>置信度阈值：</source>
        <translation>Confidence Threshold:</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="109"/>
        <source>低于此置信度的检测结果将被过滤。值越高，保留的结果越少但越准确。</source>
        <translation>Detection results below this confidence will be filtered out. Higher values retain fewer but more accurate results.</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="116"/>
        <source>重叠过滤阈值（NMS）：</source>
        <translation>Overlap Filter Threshold (NMS):</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="122"/>
        <source>重叠度过高的检测框将被合并。值越低，重叠框越少；值越高，保留更多可能重叠的框。</source>
        <translation>Detection boxes with high overlap will be merged. Lower values result in fewer overlapping boxes; higher values retain more potentially overlapping boxes.</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="154"/>
        <source>选择图像文件夹</source>
        <translation>Select Image Folder</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="193"/>
        <source>图像尺寸过大（%1×%2），最大支持 %3×%3</source>
        <translation>Image size too large (%1×%2), maximum supported %3×%3</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="200"/>
        <source>图像尺寸过小（%1×%2），最小支持 %3×%3</source>
        <translation>Image size too small (%1×%2), minimum supported %3×%3</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/inference_page.cpp" line="205"/>
        <source>当前图像：%1</source>
        <translation>Current Image: %1</translation>
    </message>
</context>
<context>
    <name>aitoolkit::ui::MainWindow</name>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="52"/>
        <source>类别数</source>
        <translation>Class Count</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="55"/>
        <source>实例数</source>
        <translation>Instance Count</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="57"/>
        <source>目标数</source>
        <translation>Object Count</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="62"/>
        <source>个类别</source>
        <translation>classes</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="65"/>
        <source>个实例</source>
        <translation>instances</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="67"/>
        <source>个目标</source>
        <translation>objects</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="74"/>
        <source>AI 检测工具 v%1</source>
        <translation>AI Detection Tool v%1</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="123"/>
        <source>任务摘要</source>
        <translation>Task Summary</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="160"/>
        <source>当前模型</source>
        <translation>Current Model</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="164"/>
        <source>当前图像</source>
        <translation>Current Image</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="168"/>
        <source>当前结果</source>
        <translation>Current Result</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="172"/>
        <source>下一步</source>
        <translation>Next Step</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="206"/>
        <location filename="../../src/ui/main_window.cpp" line="223"/>
        <source>下载示例模型</source>
        <translation>Download Sample Model</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="207"/>
        <source>未找到下载脚本。请手动执行：

powershell -ExecutionPolicy Bypass -File scripts/download_sample_model.ps1</source>
        <translation>Download script not found. Please run manually:

powershell -ExecutionPolicy Bypass -File scripts/download_sample_model.ps1</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="224"/>
        <source>即将下载 YOLOv8n 模型（约 6MB），请稍候...</source>
        <translation>Downloading YOLOv8n model (approx. 6MB), please wait...</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="233"/>
        <source>示例模型下载完成并已加载</source>
        <translation>Sample model downloaded and loaded successfully</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="237"/>
        <location filename="../../src/ui/main_window.cpp" line="273"/>
        <location filename="../../src/ui/main_window.cpp" line="297"/>
        <source>下载失败</source>
        <translation>Download Failed</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="238"/>
        <source>模型下载未完成。请检查网络连接后重试，或手动下载模型文件。</source>
        <translation>Model download incomplete. Please check your network connection and retry, or download the model file manually.</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="267"/>
        <source>正在下载 %1...</source>
        <translation>Downloading %1...</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="273"/>
        <source>未找到下载脚本。</source>
        <translation>Download script not found.</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="295"/>
        <source>模型下载完成并已加载</source>
        <translation>Model downloaded and loaded successfully</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="298"/>
        <source>模型下载未完成。请检查网络连接后重试。</source>
        <translation>Model download incomplete. Please check your network connection and retry.</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="321"/>
        <location filename="../../src/ui/main_window.cpp" line="758"/>
        <source>选择图像</source>
        <translation>Select Image</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="323"/>
        <source>图像 (*.png *.jpg *.jpeg *.bmp *.tiff *.tif *.webp)</source>
        <translation>Images (*.png *.jpg *.jpeg *.bmp *.tiff *.tif *.webp)</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="372"/>
        <source>大视频预警</source>
        <translation>Large Video Warning</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="373"/>
        <source>该视频共有 %1 帧，处理可能需要较长时间。是否继续？</source>
        <translation>This video has %1 frames. Processing may take a long time. Continue?</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="389"/>
        <source>已取消</source>
        <translation>Cancelled</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="390"/>
        <source>可重新开始检测。</source>
        <translation>You can restart detection.</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="397"/>
        <location filename="../../src/ui/main_window.cpp" line="423"/>
        <location filename="../../src/ui/main_window.cpp" line="456"/>
        <location filename="../../src/ui/main_window.cpp" line="770"/>
        <source>暂无结果</source>
        <translation>No Results Yet</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="397"/>
        <location filename="../../src/ui/main_window.cpp" line="770"/>
        <source>请先完成一次推理，再导出结果。</source>
        <translation>Please complete an inference before exporting results.</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="408"/>
        <location filename="../../src/ui/main_window.cpp" line="779"/>
        <source>导出 JSON</source>
        <translation>Export JSON</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="419"/>
        <source>JSON 已导出至 %1</source>
        <translation>JSON exported to %1</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="423"/>
        <source>请先完成一次推理，再导出图片。</source>
        <translation>Please complete an inference before exporting the image.</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="429"/>
        <source>图像不可用</source>
        <translation>Image Unavailable</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="429"/>
        <source>当前图像无法读取，无法导出渲染图。</source>
        <translation>The current image cannot be read. Unable to export the rendered image.</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="440"/>
        <source>导出图片</source>
        <translation>Export Image</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="451"/>
        <source>图片已导出至 %1</source>
        <translation>Image exported to %1</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="456"/>
        <source>没有可导出的批量结果。</source>
        <translation>No batch results available for export.</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="463"/>
        <source>批量导出 JSON</source>
        <translation>Batch Export JSON</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="474"/>
        <source>批量结果已导出至 %1</source>
        <translation>Batch results exported to %1</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="476"/>
        <source>导出失败</source>
        <translation>Export Failed</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="476"/>
        <source>无法写入文件：%1</source>
        <translation>Unable to write file: %1</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="510"/>
        <source>语言切换</source>
        <translation>Language Switch</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="511"/>
        <source>语言设置已保存，重启应用后生效。</source>
        <translation>Language settings saved. Restart the application to apply changes.</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="537"/>
        <source>推理中…</source>
        <translation>Inferring…</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="538"/>
        <source>请等待推理完成。</source>
        <translation>Please wait for inference to complete.</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="539"/>
        <source>正在推理...</source>
        <translation>Inferring...</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="550"/>
        <source>推理完成 | %1：%2 | 耗时：%3 ms</source>
        <translation>Inference Complete | %1: %2 | Time: %3 ms</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="580"/>
        <source>批量推理完成 | 图像数：%1 | 平均耗时：%2 ms | 总耗时：%3 ms</source>
        <translation>Batch Inference Complete | Images: %1 | Avg Time: %2 ms | Total Time: %3 ms</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="587"/>
        <source>批量推理完成</source>
        <translation>Batch Inference Complete</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="588"/>
        <source>共处理 %1 张图像，得到 %2 %3，总耗时 %4 ms。</source>
        <translation>Processed %1 images, found %2 %3, total time %4 ms.</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="597"/>
        <source>无帧数据</source>
        <translation>No Frame Data</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="597"/>
        <source>未能从视频中读取任何帧。</source>
        <translation>Could not read any frames from the video.</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="626"/>
        <source>视频推理完成 | 帧数：%1 | 平均耗时：%2 ms | FPS：%3 | 总耗时：%4 ms</source>
        <translation>Video Inference Complete | Frames: %1 | Avg Time: %2 ms | FPS: %3 | Total Time: %4 ms</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="634"/>
        <source>视频推理完成</source>
        <translation>Video Inference Complete</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="635"/>
        <source>共处理 %1 帧，得到 %2 %3，总耗时 %4 ms。</source>
        <translation>Processed %1 frames, found %2 %3, total time %4 ms.</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="644"/>
        <source>推理 %1/%2…</source>
        <translation>Inferring %1/%2…</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="649"/>
        <location filename="../../src/ui/main_window.cpp" line="652"/>
        <source>推理失败</source>
        <translation>Inference Failed</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="650"/>
        <source>请检查模型和输入后重试。</source>
        <translation>Please check the model and input, then retry.</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="651"/>
        <source>推理失败 | %1</source>
        <translation>Inference Failed | %1</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="674"/>
        <source>已加载：%1</source>
        <translation>Loaded: %1</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="675"/>
        <source>未选择模型清单</source>
        <translation>No Model Manifest Selected</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="677"/>
        <source>已选择：%1</source>
        <translation>Selected: %1</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="679"/>
        <source>当前结果来自视频或批量推理</source>
        <translation>Current result is from video or batch inference</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="680"/>
        <source>未选择图像</source>
        <translation>No Image Selected</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="683"/>
        <source>已完成，共 %1 %2，耗时 %3 ms</source>
        <translation>Completed, %1 %2 found, time %3 ms</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="687"/>
        <source>尚未执行检测</source>
        <translation>Detection Not Yet Run</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="690"/>
        <source>请先加载模型清单。</source>
        <translation>Please load a model manifest first.</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="692"/>
        <location filename="../../src/ui/main_window.cpp" line="698"/>
        <source>可查看结果明细，或直接导出 JSON。</source>
        <translation>You can view result details or export JSON directly.</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="694"/>
        <source>请选择一张待推理图像。</source>
        <translation>Please select an image for inference.</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="696"/>
        <source>模型和图像已就绪，可以开始检测。</source>
        <translation>Model and image are ready. You can start detection.</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="726"/>
        <source>确认退出</source>
        <translation>Confirm Exit</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="727"/>
        <source>推理正在进行中，退出将丢失当前进度。是否确认退出？</source>
        <translation>Inference is in progress. Exiting will lose current progress. Confirm exit?</translation>
    </message>
    <message>
        <location filename="../../src/ui/main_window.cpp" line="823"/>
        <location filename="../../src/ui/main_window.cpp" line="838"/>
        <source>推理进行中，请等待完成后再操作。</source>
        <translation>Inference in progress. Please wait until it completes.</translation>
    </message>
</context>
<context>
    <name>aitoolkit::ui::ModelCatalogDialog</name>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="21"/>
        <source>YOLO11 Nano — 最新一代检测模型，极速推理（约 5MB），mAP 39.5</source>
        <translation>YOLO11 Nano — Latest generation detection model, ultra-fast inference (approx. 5MB), mAP 39.5</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="30"/>
        <source>YOLO11 Small — 最新一代，精度与速度平衡（约 18MB），mAP 47.0</source>
        <translation>YOLO11 Small — Latest generation, balanced accuracy and speed (approx. 18MB), mAP 47.0</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="39"/>
        <source>YOLO11 Medium — 最新一代高精度检测（约 40MB），mAP 51.5</source>
        <translation>YOLO11 Medium — Latest generation high-accuracy detection (approx. 40MB), mAP 51.5</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="48"/>
        <source>YOLOv8 Nano — 经典检测模型，极速推理（约 6MB），mAP 37.3</source>
        <translation>YOLOv8 Nano — Classic detection model, ultra-fast inference (approx. 6MB), mAP 37.3</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="57"/>
        <source>YOLOv8 Small — 经典检测，精度与速度平衡（约 22MB），mAP 44.9</source>
        <translation>YOLOv8 Small — Classic detection, balanced accuracy and speed (approx. 22MB), mAP 44.9</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="66"/>
        <source>YOLOv8 Medium — 经典高精度检测（约 52MB），mAP 50.2</source>
        <translation>YOLOv8 Medium — Classic high-accuracy detection (approx. 52MB), mAP 50.2</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="75"/>
        <source>YOLOv5 Nano — YOLOv5 架构轻量版（约 5MB），mAP 37.1</source>
        <translation>YOLOv5 Nano — YOLOv5 architecture lightweight edition (approx. 5MB), mAP 37.1</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="84"/>
        <source>YOLO11 分类 Nano — 最新一代图像分类，极速推理（约 5MB），Top-1 69.0%</source>
        <translation>YOLO11 Classification Nano — Latest generation image classification, ultra-fast inference (approx. 5MB), Top-1 69.0%</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="93"/>
        <source>YOLO11 分类 Small — 最新一代分类，平衡精度（约 11MB），Top-1 75.4%</source>
        <translation>YOLO11 Classification Small — Latest generation classification, balanced accuracy (approx. 11MB), Top-1 75.4%</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="102"/>
        <source>YOLOv8 分类 Nano — 经典图像分类，极速推理（约 6MB），Top-1 66.6%</source>
        <translation>YOLOv8 Classification Nano — Classic image classification, ultra-fast inference (approx. 6MB), Top-1 66.6%</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="111"/>
        <source>YOLO11 分割 Nano — 最新一代实例分割，极速推理（约 6MB），mAP 38.0</source>
        <translation>YOLO11 Segmentation Nano — Latest generation instance segmentation, ultra-fast inference (approx. 6MB), mAP 38.0</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="120"/>
        <source>YOLO11 分割 Small — 最新一代分割，平衡精度（约 19MB），mAP 45.2</source>
        <translation>YOLO11 Segmentation Small — Latest generation segmentation, balanced accuracy (approx. 19MB), mAP 45.2</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="129"/>
        <source>YOLOv8 分割 Nano — 经典实例分割，极速推理（约 6MB），mAP 36.7</source>
        <translation>YOLOv8 Segmentation Nano — Classic instance segmentation, ultra-fast inference (approx. 6MB), mAP 36.7</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="138"/>
        <source>YOLOv8 分割 Small — 经典分割，平衡精度（约 23MB），mAP 44.6</source>
        <translation>YOLOv8 Segmentation Small — Classic segmentation, balanced accuracy (approx. 23MB), mAP 44.6</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="149"/>
        <source>模型目录</source>
        <translation>Model Catalog</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="154"/>
        <source>选择要下载的模型：</source>
        <translation>Select models to download:</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="158"/>
        <source>注意：Ultralytics YOLO 模型采用 AGPL-3.0 许可证。商业使用需获取 Ultralytics 商业许可，详见 &lt;a href=&quot;https://ultralytics.com/license&quot;&gt;ultralytics.com/license&lt;/a&gt;</source>
        <translation>Note: Ultralytics YOLO models use the AGPL-3.0 license. Commercial use requires an Ultralytics commercial license. See &lt;a href=&quot;https://ultralytics.com/license&quot;&gt;ultralytics.com/license&lt;/a&gt; for details.</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="167"/>
        <source>任务类型：</source>
        <translation>Task Type:</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="169"/>
        <source>全部</source>
        <translation>All</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="170"/>
        <source>目标检测</source>
        <translation>Object Detection</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="171"/>
        <source>图像分类</source>
        <translation>Image Classification</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="172"/>
        <source>实例分割</source>
        <translation>Instance Segmentation</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="185"/>
        <source>下载</source>
        <translation>Download</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="189"/>
        <source>取消</source>
        <translation>Cancel</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="230"/>
        <source>分割</source>
        <translation>Segmentation</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="232"/>
        <source>分类</source>
        <translation>Classification</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/model_catalog_dialog.cpp" line="233"/>
        <source>检测</source>
        <translation>Detection</translation>
    </message>
</context>
<context>
    <name>aitoolkit::ui::ModelsPage</name>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="21"/>
        <source>选择一个模型后，可在这里查看模型名称、输入尺寸、后端类型和标签数量。</source>
        <translation>After selecting a model, you can view the model name, input size, backend type, and label count here.</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="30"/>
        <location filename="../../src/ui/pages/models_page.cpp" line="102"/>
        <source>实例分割</source>
        <translation>Instance Segmentation</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="32"/>
        <location filename="../../src/ui/pages/models_page.cpp" line="101"/>
        <source>图像分类</source>
        <translation>Image Classification</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="33"/>
        <location filename="../../src/ui/pages/models_page.cpp" line="100"/>
        <source>目标检测</source>
        <translation>Object Detection</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="35"/>
        <source>模型名称：%1
任务类型：%2
推理后端：%3
解码器：%4
输入尺寸：%5 x %6
标签数量：%7
模型文件：%8</source>
        <translation>Model Name: %1
Task Type: %2
Inference Backend: %3
Decoder: %4
Input Size: %5 x %6
Label Count: %7
Model File: %8</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="40"/>
        <source>无（分类模型）</source>
        <translation>None (Classification Model)</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="51"/>
        <source>YOLOv8n — 目标检测</source>
        <translation>YOLOv8n — Object Detection</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="54"/>
        <source>YOLOv8 Nano 检测模型 — COCO 80 类，极速推理（约 12MB），mAP 37.3</source>
        <translation>YOLOv8 Nano Detection Model — COCO 80 classes, ultra-fast inference (approx. 12MB), mAP 37.3</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="55"/>
        <source>YOLOv8s — 目标检测</source>
        <translation>YOLOv8s — Object Detection</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="58"/>
        <source>YOLOv8 Small 检测模型 — COCO 80 类，精度更高（约 43MB），mAP 44.9</source>
        <translation>YOLOv8 Small Detection Model — COCO 80 classes, higher accuracy (approx. 43MB), mAP 44.9</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="59"/>
        <source>YOLOv8n — 图像分类</source>
        <translation>YOLOv8n — Image Classification</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="62"/>
        <source>YOLOv8 Nano 分类模型 — ImageNet 1000 类，极速推理（约 10MB），Top-1 66.6%</source>
        <translation>YOLOv8 Nano Classification Model — ImageNet 1000 classes, ultra-fast inference (approx. 10MB), Top-1 66.6%</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="63"/>
        <source>YOLOv8n — 实例分割</source>
        <translation>YOLOv8n — Instance Segmentation</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="66"/>
        <source>YOLOv8 Nano 分割模型 — COCO 80 类实例分割（约 13MB），mAP 36.7</source>
        <translation>YOLOv8 Nano Segmentation Model — COCO 80 classes instance segmentation (approx. 13MB), mAP 36.7</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="67"/>
        <source>YOLOv8s — 实例分割</source>
        <translation>YOLOv8s — Instance Segmentation</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="70"/>
        <source>YOLOv8 Small 分割模型 — COCO 80 类实例分割（约 45MB），mAP 44.6</source>
        <translation>YOLOv8 Small Segmentation Model — COCO 80 classes instance segmentation (approx. 45MB), mAP 44.6</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="80"/>
        <source>模型选择</source>
        <translation>Model Selection</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="83"/>
        <source>选择一个内置模型即可开始推理，也可导入自定义模型。</source>
        <translation>Select a built-in model to start inference, or import a custom model.</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="93"/>
        <source>内置模型</source>
        <translation>Built-in Models</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="97"/>
        <source>任务类型：</source>
        <translation>Task Type:</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="99"/>
        <source>全部</source>
        <translation>All</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="116"/>
        <source>使用此模型</source>
        <translation>Use This Model</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="132"/>
        <source>自定义模型</source>
        <translation>Custom Model</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="136"/>
        <source>支持导入 JSON 模型清单文件或直接导入 ONNX 模型文件。</source>
        <translation>Supports importing JSON model manifest files or directly importing ONNX model files.</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="141"/>
        <source>导入模型清单</source>
        <translation>Import Model Manifest</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="143"/>
        <source>导入 ONNX 文件</source>
        <translation>Import ONNX File</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="159"/>
        <source>当前模型</source>
        <translation>Current Model</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="162"/>
        <location filename="../../src/ui/pages/models_page.cpp" line="318"/>
        <source>未选择模型</source>
        <translation>No Model Selected</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="179"/>
        <source>标签列表</source>
        <translation>Label List</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="212"/>
        <source>选择模型清单</source>
        <translation>Select Model Manifest</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="223"/>
        <source>选择 ONNX 文件</source>
        <translation>Select ONNX File</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="277"/>
        <source>

⚠ 模型文件未找到，请确认 models 目录中包含所需文件。</source>
        <translation>

⚠ Model file not found. Please ensure the models directory contains the required files.</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/models_page.cpp" line="325"/>
        <source>当前模型：%1</source>
        <translation>Current Model: %1</translation>
    </message>
</context>
<context>
    <name>aitoolkit::ui::NavPanel</name>
    <message>
        <location filename="../../src/ui/nav_panel.cpp" line="27"/>
        <source>AI 检测工具</source>
        <translation>AI Detection Tool</translation>
    </message>
    <message>
        <location filename="../../src/ui/nav_panel.cpp" line="29"/>
        <source>基于 ONNX Runtime 的轻量级 AI 推理桌面工具，支持目标检测、图像分类和实例分割。</source>
        <translation>A lightweight AI inference desktop tool based on ONNX Runtime, supporting object detection, image classification, and instance segmentation.</translation>
    </message>
    <message>
        <location filename="../../src/ui/nav_panel.cpp" line="37"/>
        <source>功能导航</source>
        <translation>Navigation</translation>
    </message>
    <message>
        <location filename="../../src/ui/nav_panel.cpp" line="47"/>
        <source>首页</source>
        <translation>Home</translation>
    </message>
    <message>
        <location filename="../../src/ui/nav_panel.cpp" line="48"/>
        <source>模型</source>
        <translation>Models</translation>
    </message>
    <message>
        <location filename="../../src/ui/nav_panel.cpp" line="49"/>
        <source>推理</source>
        <translation>Inference</translation>
    </message>
    <message>
        <location filename="../../src/ui/nav_panel.cpp" line="50"/>
        <source>结果</source>
        <translation>Results</translation>
    </message>
    <message>
        <location filename="../../src/ui/nav_panel.cpp" line="51"/>
        <source>设置</source>
        <translation>Settings</translation>
    </message>
    <message>
        <location filename="../../src/ui/nav_panel.cpp" line="69"/>
        <source>支持 YOLOv5/v8/v11 系列模型，更多功能持续更新中。</source>
        <translation>Supports YOLOv5/v8/v11 series models. More features coming soon.</translation>
    </message>
</context>
<context>
    <name>aitoolkit::ui::ResultsPage</name>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="30"/>
        <source>结果 %1</source>
        <translation>Result %1</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="113"/>
        <source>结果</source>
        <translation>Results</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="116"/>
        <location filename="../../src/ui/pages/results_page.cpp" line="552"/>
        <source>导出 JSON</source>
        <translation>Export JSON</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="119"/>
        <location filename="../../src/ui/pages/results_page.cpp" line="557"/>
        <source>导出图片</source>
        <translation>Export Image</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="122"/>
        <source>批量导出 JSON</source>
        <translation>Batch Export JSON</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="133"/>
        <location filename="../../src/ui/pages/results_page.cpp" line="219"/>
        <location filename="../../src/ui/pages/results_page.cpp" line="278"/>
        <source>当前还没有推理结果</source>
        <translation>No inference results yet</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="143"/>
        <location filename="../../src/ui/pages/results_page.cpp" line="283"/>
        <location filename="../../src/ui/pages/results_page.cpp" line="428"/>
        <location filename="../../src/ui/pages/results_page.cpp" line="464"/>
        <location filename="../../src/ui/pages/results_page.cpp" line="487"/>
        <source>全部类别</source>
        <translation>All Categories</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="176"/>
        <source>类别筛选：</source>
        <translation>Category Filter:</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="299"/>
        <source>一次导出当前批量结果的全部 JSON</source>
        <translation>Export all JSON for the current batch results at once</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="323"/>
        <source>类别数：%1</source>
        <translation>Classes: %1</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="325"/>
        <source>实例数：%1</source>
        <translation>Instances: %1</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="326"/>
        <source>目标数：%1</source>
        <translation>Objects: %1</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="330"/>
        <source>第 %1 / %2 项 | </source>
        <translation>Item %1 / %2 | </translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="334"/>
        <source>图像：%1×%2</source>
        <translation>Image: %1×%2</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="335"/>
        <source>来源：%1</source>
        <translation>Source: %1</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="337"/>
        <source>%1模型：%2 | %3 | %4 | 耗时：%5 ms</source>
        <translation>%1Model: %2 | %3 | %4 | Time: %5 ms</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="416"/>
        <source>类别</source>
        <translation>Category</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="417"/>
        <location filename="../../src/ui/pages/results_page.cpp" line="436"/>
        <source>标签</source>
        <translation>Label</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="418"/>
        <location filename="../../src/ui/pages/results_page.cpp" line="437"/>
        <source>置信度</source>
        <translation>Confidence</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="419"/>
        <source>框选范围</source>
        <translation>Bounding Box</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="435"/>
        <source>排名</source>
        <translation>Rank</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="535"/>
        <source>请先完成一次推理，再导出图片。</source>
        <translation>Please complete an inference before exporting the image.</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="540"/>
        <source>当前结果没有可导出的预览图，请先选择可读取的图像结果。</source>
        <translation>The current result has no exportable preview image. Please select a result with a readable image.</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="546"/>
        <source>导出当前选中结果的渲染图片</source>
        <translation>Export rendered image of the selected result</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="547"/>
        <source>导出当前结果的渲染图片</source>
        <translation>Export rendered image of the current result</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="552"/>
        <source>导出当前 JSON</source>
        <translation>Export Current JSON</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="554"/>
        <source>导出当前选中结果的 JSON</source>
        <translation>Export JSON of the selected result</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="554"/>
        <source>导出当前结果的 JSON</source>
        <translation>Export JSON of the current result</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="557"/>
        <source>导出当前图片</source>
        <translation>Export Current Image</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/results_page.cpp" line="560"/>
        <source>导出全部 JSON</source>
        <translation>Export All JSON</translation>
    </message>
</context>
<context>
    <name>aitoolkit::ui::SettingsPage</name>
    <message>
        <location filename="../../src/ui/pages/settings_page.cpp" line="25"/>
        <source>设置</source>
        <translation>Settings</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/settings_page.cpp" line="28"/>
        <source>默认导出目录</source>
        <translation>Default Export Directory</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/settings_page.cpp" line="31"/>
        <source>未设置时默认使用当前图像所在目录</source>
        <translation>Defaults to the directory of the current image if not set</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/settings_page.cpp" line="32"/>
        <source>浏览</source>
        <translation>Browse</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/settings_page.cpp" line="37"/>
        <source>推理线程数</source>
        <translation>Inference Threads</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/settings_page.cpp" line="41"/>
        <source>ONNX Runtime 推理使用的 CPU 线程数。增加线程数可利用多核 CPU 加速推理。</source>
        <translation>Number of CPU threads used for ONNX Runtime inference. Increasing threads can leverage multi-core CPUs to accelerate inference.</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/settings_page.cpp" line="46"/>
        <source>使用 GPU 加速推理（CUDA）</source>
        <translation>Use GPU Acceleration for Inference (CUDA)</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/settings_page.cpp" line="47"/>
        <source>需要 NVIDIA GPU 和 CUDA 驱动。如果 GPU 不可用，将自动回退到 CPU 推理。</source>
        <translation>Requires an NVIDIA GPU and CUDA driver. If GPU is unavailable, it will automatically fall back to CPU inference.</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/settings_page.cpp" line="50"/>
        <source>界面语言</source>
        <translation>Interface Language</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/settings_page.cpp" line="52"/>
        <source>跟随系统</source>
        <translation>Follow System</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/settings_page.cpp" line="59"/>
        <source>最近模型</source>
        <translation>Recent Models</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/settings_page.cpp" line="64"/>
        <source>最近图像</source>
        <translation>Recent Images</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/settings_page.cpp" line="91"/>
        <source>选择默认导出目录</source>
        <translation>Select Default Export Directory</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/settings_page.cpp" line="101"/>
        <source>关于</source>
        <translation>About</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/settings_page.cpp" line="106"/>
        <source>关于 AI 检测工具</source>
        <translation>About AI Detection Tool</translation>
    </message>
    <message>
        <location filename="../../src/ui/pages/settings_page.cpp" line="107"/>
        <source>&lt;h3&gt;AI 检测工具 v%1&lt;/h3&gt;&lt;p&gt;基于 ONNX Runtime 的轻量级目标检测桌面应用&lt;/p&gt;&lt;p&gt;支持 YOLOv5/YOLOv8/YOLOX 等模型&lt;/p&gt;&lt;p&gt;推理后端：%2&lt;/p&gt;&lt;p&gt;&amp;copy; 2026 MyProject&lt;/p&gt;</source>
        <translation>&lt;h3&gt;AI Detection Tool v%1&lt;/h3&gt;&lt;p&gt;A lightweight object detection desktop application based on ONNX Runtime&lt;/p&gt;&lt;p&gt;Supports YOLOv5/YOLOv8/YOLOX and other models&lt;/p&gt;&lt;p&gt;Inference Backend: %2&lt;/p&gt;&lt;p&gt;&amp;copy; 2026 MyProject&lt;/p&gt;</translation>
    </message>
</context>
<context>
    <name>aitoolkit::ui::dialogs::OnnxSetupDialog</name>
    <message>
        <location filename="../../src/ui/dialogs/onnx_setup_dialog.cpp" line="17"/>
        <source>配置 ONNX 模型</source>
        <translation>Configure ONNX Model</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/onnx_setup_dialog.cpp" line="48"/>
        <source>每行一个标签，可留空</source>
        <translation>One label per line, can be left empty</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/onnx_setup_dialog.cpp" line="51"/>
        <source>模型名称：</source>
        <translation>Model Name:</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/onnx_setup_dialog.cpp" line="52"/>
        <source>输入宽度：</source>
        <translation>Input Width:</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/onnx_setup_dialog.cpp" line="53"/>
        <source>输入高度：</source>
        <translation>Input Height:</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/onnx_setup_dialog.cpp" line="54"/>
        <source>置信度阈值：</source>
        <translation>Confidence Threshold:</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/onnx_setup_dialog.cpp" line="55"/>
        <source>NMS 阈值：</source>
        <translation>NMS Threshold:</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/onnx_setup_dialog.cpp" line="56"/>
        <source>标签列表：</source>
        <translation>Label List:</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/onnx_setup_dialog.cpp" line="67"/>
        <source>ONNX 文件：%1</source>
        <translation>ONNX File: %1</translation>
    </message>
    <message>
        <location filename="../../src/ui/dialogs/onnx_setup_dialog.cpp" line="69"/>
        <source>当前 ONNX 导入仅生成 detection 模型清单；classification / segmentation 请使用现成 JSON 清单。</source>
        <translation>Current ONNX import only generates detection model manifests; for classification/segmentation, please use an existing JSON manifest.</translation>
    </message>
</context>
</TS>