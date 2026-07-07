import SwiftUI
import UniformTypeIdentifiers

struct ContentView: View {
  @Environment(AppModel.self) private var appModel
  @Environment(\.openImmersiveSpace) private var openImmersiveSpace
  @State private var showImporter = false

  var body: some View {
    NavigationStack {
      List(appModel.rooms) { room in
        Button(room.displayName) {
          appModel.activeRoomId = room.id
          Task { await openImmersiveSpace(id: "RoomView") }
        }
      }
      .navigationTitle("SOS Vision")
      .toolbar {
        ToolbarItemGroup(placement: .topBarTrailing) {
          Button("Scan") { appModel.beginScan() }
          Button("Stop") { appModel.stopScanAndSave() }
          Button("Import") { showImporter = true }
        }
      }
      .overlay(alignment: .bottom) {
        Text(appModel.alignmentStatus)
          .font(.caption)
          .padding(8)
      }
      .fileImporter(isPresented: $showImporter, allowedContentTypes: [.data], allowsMultipleSelection: false) { result in
        guard case .success(let urls) = result, let url = urls.first, url.startAccessingSecurityScopedResource() else { return }
        defer { url.stopAccessingSecurityScopedResource() }
        if let roomId = SOSCoreBridge.importSplat(path: url.path) {
          appModel.activeRoomId = roomId
          appModel.refreshRooms()
        }
      }
      .onAppear { appModel.refreshRooms() }
    }
  }
}
