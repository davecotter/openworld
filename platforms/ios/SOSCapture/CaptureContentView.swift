import SwiftUI
import RoomPlan

@main
struct SOSCaptureApp: App {
  var body: some Scene {
    WindowGroup {
      CaptureContentView()
    }
  }
}

struct CaptureContentView: View {
  @State private var roomName = "Captured Room"
  @State private var status = "Ready"
  @State private var exportedPath = ""

  var body: some View {
    NavigationStack {
      Form {
        TextField("Room name", text: $roomName)
        Button("Scan with RoomPlan") {
          status = SOSCaptureBridge.startScan(displayName: roomName) ?? "Scan failed"
        }
        Button("Import Splat File") {
          status = "Use Files app → Open In SOS Capture (Phase 0 stub)"
        }
        Button("Export .sosroom Bundle") {
          exportedPath = SOSCaptureBridge.exportLatestBundle() ?? ""
          status = exportedPath.isEmpty ? "Nothing to export" : "Exported"
        }
        if !exportedPath.isEmpty {
          Text(exportedPath).font(.caption)
        }
        Text(status).foregroundStyle(.secondary)
      }
      .navigationTitle("SOS Capture")
    }
  }
}
