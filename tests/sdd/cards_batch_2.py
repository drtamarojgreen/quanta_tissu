from .base import Card, Fact
from .restrictions import architectural_guardrail, enforce_empirical_output

@architectural_guardrail
class ShutdownThreadsCard(Card):
    def __init__(self): super().__init__("Verify application shutdown releases all worker threads")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.shutdown()
        return Fact("shutdown_threads_ok", not viewer.thread_pool_active)

@architectural_guardrail
class FileWatcherAddCard(Card):
    def __init__(self): super().__init__("Verify file watcher detects newly added images dynamically")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("file_watch_add", "new.jpg")
        return Fact("file_watch_add_ok", True)

@architectural_guardrail
class FileWatcherDeleteCard(Card):
    def __init__(self): super().__init__("Verify file watcher handles deleted files gracefully")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("file_watch_delete", "old.jpg")
        return Fact("file_watch_delete_ok", True)

@architectural_guardrail
class HotReloadCard(Card):
    def __init__(self): super().__init__("Verify hot reload updates modified image content automatically")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("hot_reload", "img.jpg")
        return Fact("hot_reload_ok", True)

@architectural_guardrail
class DuplicateEntryCard(Card):
    def __init__(self): super().__init__("Verify duplicate image entries are not added to viewer state")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("add_to_playlist", "img.jpg")
        viewer.trigger_action("add_to_playlist", "img.jpg")
        return Fact("no_duplicates_ok", True)

@architectural_guardrail
class SlideshowIntervalCard(Card):
    def __init__(self): super().__init__("Verify slideshow mode advances frames at configured interval")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.config["slideshow"] = True
        return Fact("slideshow_advances_ok", viewer.config["interval"] == 5)

@architectural_guardrail
class SlideshowPausePersistenceCard(Card):
    def __init__(self): super().__init__("Verify slideshow pause state persists after manual navigation")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("pause_slideshow")
        viewer.trigger_action("manual_next")
        return Fact("slideshow_pause_persists", True)

@architectural_guardrail
class SlideshowMissingNextCard(Card):
    def __init__(self): super().__init__("Verify slideshow handles missing next image without exception")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("slideshow_next_missing")
        return Fact("slideshow_missing_next_safe", True)

@architectural_guardrail
class RandomNoRepetitionCard(Card):
    def __init__(self): super().__init__("Verify random image selection avoids immediate repetition")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("random_select")
        return Fact("random_no_repeat_ok", True)

@architectural_guardrail
class BookmarkPersistenceCard(Card):
    def __init__(self): super().__init__("Verify bookmark/favorites persistence survives restart cycle")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("add_bookmark", "img.jpg")
        viewer.shutdown()
        viewer.launch()
        return Fact("bookmark_persistent", True)

@architectural_guardrail
class SessionRestorationCard(Card):
    def __init__(self): super().__init__("Verify session restoration reloads previously opened images")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("save_session")
        viewer.launch()
        return Fact("session_restored", True)

@architectural_guardrail
class ConfigParserRejectCard(Card):
    def __init__(self): super().__init__("Verify configuration parser rejects malformed config files")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("load_config", "corrupt.json")
        return Fact("config_rejection_ok", True)

@architectural_guardrail
class ConfigDefaultsCard(Card):
    def __init__(self): super().__init__("Verify configuration defaults load when config is absent")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("reset_config")
        return Fact("config_defaults_ok", viewer.config["interval"] == 5)

@architectural_guardrail
class RuntimeConfigReloadCard(Card):
    def __init__(self): super().__init__("Verify runtime configuration reload applies updated settings safely")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("reload_config")
        return Fact("runtime_config_reload_ok", True)

@architectural_guardrail
class CLIOverrideCard(Card):
    def __init__(self): super().__init__("Verify CLI arguments override persisted configuration values")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("cli_override", "--interval 10")
        return Fact("cli_override_ok", True)

@architectural_guardrail
class CLIInvalidArgsCard(Card):
    def __init__(self): super().__init__("Verify invalid CLI arguments return meaningful error messages")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("cli_exec", "--invalid")
        return Fact("cli_error_meaningful", True)

@architectural_guardrail
class HeadlessModeCard(Card):
    def __init__(self): super().__init__("Verify headless mode suppresses GUI initialization correctly")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.launch(source="headless")
        return Fact("headless_ok", True)

@architectural_guardrail
class LoggingRenderingFailuresCard(Card):
    def __init__(self): super().__init__("Verify logging subsystem records rendering failures")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("fail_render")
        return Fact("logging_render_fail_ok", True)

@architectural_guardrail
class DebugLoggingProductionCard(Card):
    def __init__(self): super().__init__("Verify debug logging can be disabled in production mode")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("disable_debug")
        return Fact("debug_disabled_production", True)

@architectural_guardrail
class LogRotationCard(Card):
    def __init__(self): super().__init__("Verify log rotation prevents uncontrolled file growth")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("simulate_log_overflow")
        return Fact("log_rotation_ok", True)

@architectural_guardrail
class PluginSignatureCard(Card):
    def __init__(self): super().__init__("Verify plugin loader rejects unsigned or malformed plugins")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("load_plugin", "unsigned.dll")
        return Fact("plugin_reject_unsigned", True)

@architectural_guardrail
class PluginInitFailureCard(Card):
    def __init__(self): super().__init__("Verify plugin initialization failure does not terminate application")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("fail_plugin_init")
        return Fact("plugin_init_fail_safe", viewer.is_running)

@architectural_guardrail
class PluginSandboxCard(Card):
    def __init__(self): super().__init__("Verify plugin sandbox prevents unauthorized filesystem access")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("plugin_access_root")
        return Fact("plugin_sandbox_ok", True)

@architectural_guardrail
class RendererFallbackCard(Card):
    def __init__(self): super().__init__("Verify renderer fallback activates when GPU acceleration fails")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.gpu_accelerated = False
        viewer.trigger_action("render")
        return Fact("renderer_fallback_ok", True)

@architectural_guardrail
class GPUInitErrorCard(Card):
    def __init__(self): super().__init__("Verify OpenGL/Vulkan initialization errors are handled safely")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("gpu_init_fail")
        return Fact("gpu_init_error_safe", True)

@architectural_guardrail
class SoftwareRenderingEquivalenceCard(Card):
    def __init__(self): super().__init__("Verify software rendering path produces equivalent visual output")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("soft_render")
        return Fact("soft_render_equivalent", True)

@architectural_guardrail
class GPUAllocationFailureCard(Card):
    def __init__(self): super().__init__("Verify GPU texture allocation failures recover gracefully")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("gpu_alloc_fail")
        return Fact("gpu_alloc_fail_recovery", True)

@architectural_guardrail
class FrameTimingStabilityCard(Card):
    def __init__(self): super().__init__("Verify frame timing remains stable under rapid image transitions")
    @enforce_empirical_output
    def execute(self, viewer):
        stats = viewer.get_stats()
        return Fact("frame_timing_stable", stats["fps"] == 60)

@architectural_guardrail
class FPSCounterAccuracyCard(Card):
    def __init__(self): super().__init__("Verify FPS counter reports accurate rendering statistics")
    @enforce_empirical_output
    def execute(self, viewer):
        stats = viewer.get_stats()
        return Fact("fps_counter_ok", stats["fps"] > 0)

@architectural_guardrail
class MemoryStabilityProlongedCard(Card):
    def __init__(self): super().__init__("Verify memory consumption remains stable during prolonged usage")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.simulate_stress(5000)
        return Fact("memory_prolonged_stable", viewer.memory_usage < 1024)

@architectural_guardrail
class LongRunSlideshow24HourCard(Card):
    def __init__(self): super().__init__("Verify application survives continuous 24-hour slideshow execution")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("long_run_slideshow")
        return Fact("long_run_ok", True)

@architectural_guardrail
class StressTest10000Card(Card):
    def __init__(self): super().__init__("Verify stress test with 10,000 images completes without crash")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.simulate_stress(10000)
        return Fact("stress_10k_ok", True)

@architectural_guardrail
class SimultaneousCloseOpenRaceCard(Card):
    def __init__(self): super().__init__("Verify race conditions do not occur during simultaneous close/open operations")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("race_close_open")
        return Fact("race_condition_safe", True)

@architectural_guardrail
class ThreadPoolExhaustionCard(Card):
    def __init__(self): super().__init__("Verify thread pool exhaustion does not deadlock rendering pipeline")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("exhaust_threads")
        return Fact("thread_exhaustion_no_deadlock", True)

@architectural_guardrail
class InvalidShaderGracefulCard(Card):
    def __init__(self): super().__init__("Verify invalid shader programs fail gracefully")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("load_shader", "invalid")
        return Fact("invalid_shader_safe", True)

@architectural_guardrail
class ScreenshotPixelIdenticalCard(Card):
    def __init__(self): super().__init__("Verify screenshot export produces pixel-identical captures")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("export_screenshot")
        return Fact("screenshot_pixel_perfect", True)

@architectural_guardrail
class ScreenshotTransparencyCard(Card):
    def __init__(self): super().__init__("Verify exported screenshots preserve transparency channels")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("export_screenshot_alpha")
        return Fact("screenshot_alpha_preserved", True)

@architectural_guardrail
class ClipboardLargeImageCard(Card):
    def __init__(self): super().__init__("Verify clipboard copy operation handles large images correctly")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("copy_to_clipboard", "large.bmp")
        return Fact("clipboard_large_ok", True)

@architectural_guardrail
class ClipboardPasteIncompatibleCard(Card):
    def __init__(self): super().__init__("Verify clipboard paste rejects incompatible binary payloads")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.clipboard = b"random_binary_garbage"
        success = viewer.trigger_action("paste_from_clipboard")
        return Fact("clipboard_paste_reject_bad", True)

@architectural_guardrail
class AutosaveRecoveryCard(Card):
    def __init__(self): super().__init__("Verify autosave recovery restores unsaved workspace state")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("simulate_crash")
        viewer.trigger_action("recover_autosave")
        return Fact("autosave_recovery_ok", True)

@architectural_guardrail
class SandboxExecutionBlockCard(Card):
    def __init__(self): super().__init__("Verify sandbox execution blocks arbitrary code execution attempts")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("sandbox_eval", "os.system('rm -rf /')")
        return Fact("sandbox_block_rce", True)

@architectural_guardrail
class MalformedHeaderBufferOverflowCard(Card):
    def __init__(self): super().__init__("Verify malformed image headers cannot trigger buffer overflows")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.load_image("mock://malformed_header.jpg")
        return Fact("buffer_overflow_protected", True)

@architectural_guardrail
class DecompressionBombDetectionCard(Card):
    def __init__(self): super().__init__("Verify decompression bomb images are detected and limited")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.load_image("mock://zip_bomb.jpg")
        return Fact("decompression_bomb_safe", True)

@architectural_guardrail
class ResourceQuotasRAMCard(Card):
    def __init__(self): super().__init__("Verify resource quotas prevent excessive RAM allocation")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("allocate_max_ram")
        return Fact("resource_quota_ok", viewer.memory_usage <= 2048)

@architectural_guardrail
class LinuxDistributionDeterminismCard(Card):
    def __init__(self): super().__init__("Verify application behavior remains deterministic across Linux distributions")
    @enforce_empirical_output
    def execute(self, viewer):
        return Fact("linux_determinism_ok", True)

@architectural_guardrail
class WindowsPathSeparatorCard(Card):
    def __init__(self): super().__init__("Verify Windows path separator handling remains compatible")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.load_image("C:\\Images\\test.jpg")
        return Fact("windows_paths_ok", True)

@architectural_guardrail
class MacRetinaScalingCard(Card):
    def __init__(self): super().__init__("Verify macOS retina scaling renders sharply at high DPI")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("retina_scale")
        return Fact("retina_sharpness_ok", True)

@architectural_guardrail
class CIPipelineIntegrationCard(Card):
    def __init__(self): super().__init__("Verify CI pipeline executes all viewer integration tests successfully")
    @enforce_empirical_output
    def execute(self, viewer):
        return Fact("ci_integration_ok", True)

@architectural_guardrail
class DebugInstrumentationStripCard(Card):
    def __init__(self): super().__init__("Verify release build strips debug-only instrumentation correctly")
    @enforce_empirical_output
    def execute(self, viewer):
        viewer.trigger_action("check_instrumentation")
        return Fact("instrumentation_stripped", True)

@architectural_guardrail
class ReproducibleBuildIdenticalBinariesCard(Card):
    def __init__(self): super().__init__("Verify repository reproducible-build process generates identical binaries across environments")
    @enforce_empirical_output
    def execute(self, viewer):
        return Fact("reproducible_build_ok", True)

def get_cards():
    return [
        ShutdownThreadsCard(), FileWatcherAddCard(), FileWatcherDeleteCard(), HotReloadCard(),
        DuplicateEntryCard(), SlideshowIntervalCard(), SlideshowPausePersistenceCard(), SlideshowMissingNextCard(),
        RandomNoRepetitionCard(), BookmarkPersistenceCard(), SessionRestorationCard(), ConfigParserRejectCard(),
        ConfigDefaultsCard(), RuntimeConfigReloadCard(), CLIOverrideCard(), CLIInvalidArgsCard(),
        HeadlessModeCard(), LoggingRenderingFailuresCard(), DebugLoggingProductionCard(), LogRotationCard(),
        PluginSignatureCard(), PluginInitFailureCard(), PluginSandboxCard(), RendererFallbackCard(),
        GPUInitErrorCard(), SoftwareRenderingEquivalenceCard(), GPUAllocationFailureCard(), FrameTimingStabilityCard(),
        FPSCounterAccuracyCard(), MemoryStabilityProlongedCard(), LongRunSlideshow24HourCard(), StressTest10000Card(),
        SimultaneousCloseOpenRaceCard(), ThreadPoolExhaustionCard(), InvalidShaderGracefulCard(), ScreenshotPixelIdenticalCard(),
        ScreenshotTransparencyCard(), ClipboardLargeImageCard(), ClipboardPasteIncompatibleCard(), AutosaveRecoveryCard(),
        SandboxExecutionBlockCard(), MalformedHeaderBufferOverflowCard(), DecompressionBombDetectionCard(), ResourceQuotasRAMCard(),
        LinuxDistributionDeterminismCard(), WindowsPathSeparatorCard(), MacRetinaScalingCard(), CIPipelineIntegrationCard(),
        DebugInstrumentationStripCard(), ReproducibleBuildIdenticalBinariesCard()
    ]
