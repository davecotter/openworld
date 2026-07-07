import SwiftUI

@main
struct SOSVisionProApp: App {
  @State private var appModel = AppModel()

  var body: some Scene {
    WindowGroup {
      ContentView()
        .environment(appModel)
    }
    .windowStyle(.plain)

    ImmersiveSpace(id: "RoomView") {
      RoomImmersiveView()
        .environment(appModel)
    }
    .immersionStyle(selection: .constant(.mixed), in: .mixed, .progressive)
  }
}
