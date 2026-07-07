#!/usr/bin/env python3
"""Generate a minimal SOSVisionPro.xcodeproj for visionOS 27."""

import pathlib
import uuid

ROOT = pathlib.Path(__file__).resolve().parent
APP = ROOT / "SOSVisionPro"
PROJ = ROOT / "SOSVisionPro.xcodeproj"
PBX = PROJ / "project.pbxproj"

swift_files = sorted(str(p.relative_to(APP)) for p in APP.rglob("*.swift"))
objc_files = sorted({str(p.relative_to(APP)) for p in list(APP.rglob("*.mm")) + list(APP.rglob("*.h"))})
plist_file = "Info.plist"


def uid():
    return uuid.uuid4().hex[:24].upper()


file_refs = {f: uid() for f in swift_files + list(objc_files) + [plist_file]}
build_swift = {f: uid() for f in swift_files}
build_objc = {f: uid() for f in objc_files}
target_id = uid()
project_id = uid()
config_list = uid()
debug_cfg = uid()
release_cfg = uid()
group_root = uid()
group_products = uid()
group_sources = uid()
product_ref = uid()
sources_phase = uid()
frameworks_phase = uid()
resources_phase = uid()

pbx = """// !$*UTF8*$!
{
  archiveVersion = 1;
  classes = {};
  objectVersion = 56;
  objects = {

/* Begin PBXBuildFile section */
"""

for f in swift_files:
    pbx += f'\t\t{build_swift[f]} /* {pathlib.Path(f).name} in Sources */ = {{isa = PBXBuildFile; fileRef = {file_refs[f]}; }};\n'
for f in objc_files:
    pbx += f'\t\t{build_objc[f]} /* {pathlib.Path(f).name} in Sources */ = {{isa = PBXBuildFile; fileRef = {file_refs[f]}; }};\n'
pbx += f"""/* End PBXBuildFile section */

/* Begin PBXFileReference section */
\t\t{product_ref} /* SOSVisionPro.app */ = {{isa = PBXFileReference; explicitFileType = wrapper.application; includeInIndex = 0; path = SOSVisionPro.app; sourceTree = BUILT_PRODUCTS_DIR; }};
"""
for f, ref in file_refs.items():
    if f.endswith(".plist"):
        typ = "text.plist.xml"
    elif f.endswith(".mm"):
        typ = "sourcecode.cpp.objcpp"
    elif f.endswith(".h"):
        typ = "sourcecode.c.h"
    else:
        typ = "sourcecode.swift"
    pbx += f'\t\t{ref} /* {pathlib.Path(f).name} */ = {{isa = PBXFileReference; lastKnownFileType = {typ}; path = "{f}"; sourceTree = "<group>"; }};\n'

pbx += f"""/* End PBXFileReference section */

/* Begin PBXFrameworksBuildPhase section */
\t\t{frameworks_phase} /* Frameworks */ = {{
\t\t\tisa = PBXFrameworksBuildPhase;
\t\t\tbuildActionMask = 2147483647;
\t\t\tfiles = (
\t\t\t);
\t\t\trunOnlyForDeploymentPostprocessing = 0;
\t\t}};
/* End PBXFrameworksBuildPhase section */

/* Begin PBXGroup section */
\t\t{group_root} = {{
\t\t\tisa = PBXGroup;
\t\t\tchildren = (
\t\t\t\t{group_sources} /* SOSVisionPro */,
\t\t\t\t{group_products} /* Products */,
\t\t\t);
\t\t\tsourceTree = "<group>";
\t\t}};
\t\t{group_products} /* Products */ = {{
\t\t\tisa = PBXGroup;
\t\t\tchildren = (
\t\t\t\t{product_ref} /* SOSVisionPro.app */,
\t\t\t);
\t\t\tname = Products;
\t\t\tsourceTree = "<group>";
\t\t}};
\t\t{group_sources} /* SOSVisionPro */ = {{
\t\t\tisa = PBXGroup;
\t\t\tchildren = (
"""
for ref in file_refs.values():
    pbx += f"\t\t\t\t{ref},\n"
pbx += f"""\t\t\t);
\t\t\tpath = SOSVisionPro;
\t\t\tsourceTree = "<group>";
\t\t}};
/* End PBXGroup section */

/* Begin PBXNativeTarget section */
\t\t{target_id} /* SOSVisionPro */ = {{
\t\t\tisa = PBXNativeTarget;
\t\t\tbuildConfigurationList = {config_list};
\t\t\tbuildPhases = (
\t\t\t\t{sources_phase} /* Sources */,
\t\t\t\t{frameworks_phase} /* Frameworks */,
\t\t\t\t{resources_phase} /* Resources */,
\t\t\t);
\t\t\tbuildRules = (
\t\t\t);
\t\t\tdependencies = (
\t\t\t);
\t\t\tname = SOSVisionPro;
\t\t\tproductName = SOSVisionPro;
\t\t\tproductReference = {product_ref};
\t\t\tproductType = "com.apple.product-type.application";
\t\t}};
/* End PBXNativeTarget section */

/* Begin PBXProject section */
\t\t{project_id} /* Project object */ = {{
\t\t\tisa = PBXProject;
\t\t\tattributes = {{ BuildIndependentTargetsInParallel = 1; LastUpgradeCheck = 1600; }};
\t\t\tbuildConfigurationList = {config_list};
\t\t\tcompatibilityVersion = "Xcode 14.0";
\t\t\tdevelopmentRegion = en;
\t\t\thasScannedForEncodings = 0;
\t\t\tknownRegions = (en, Base);
\t\t\tmainGroup = {group_root};
\t\t\tproductRefGroup = {group_products};
\t\t\tprojectDirPath = "";
\t\t\tprojectRoot = "";
\t\t\ttargets = (
\t\t\t\t{target_id} /* SOSVisionPro */,
\t\t\t);
\t\t}};
/* End PBXProject section */

/* Begin PBXResourcesBuildPhase section */
\t\t{resources_phase} /* Resources */ = {{
\t\t\tisa = PBXResourcesBuildPhase;
\t\t\tbuildActionMask = 2147483647;
\t\t\tfiles = (
\t\t\t);
\t\t\trunOnlyForDeploymentPostprocessing = 0;
\t\t}};
/* End PBXResourcesBuildPhase section */

/* Begin PBXSourcesBuildPhase section */
\t\t{sources_phase} /* Sources */ = {{
\t\t\tisa = PBXSourcesBuildPhase;
\t\t\tbuildActionMask = 2147483647;
\t\t\tfiles = (
"""
for f in swift_files:
    pbx += f"\t\t\t\t{build_swift[f]} /* {pathlib.Path(f).name} in Sources */,\n"
for f in objc_files:
    pbx += f"\t\t\t\t{build_objc[f]} /* {pathlib.Path(f).name} in Sources */,\n"
pbx += f"""\t\t\t);
\t\t\trunOnlyForDeploymentPostprocessing = 0;
\t\t}};
/* End PBXSourcesBuildPhase section */

/* Begin XCBuildConfiguration section */
\t\t{debug_cfg} /* Debug */ = {{
\t\t\tisa = XCBuildConfiguration;
\t\t\tbuildSettings = {{
\t\t\t\tCODE_SIGN_STYLE = Automatic;
\t\t\t\tCURRENT_PROJECT_VERSION = 1;
\t\t\t\tGENERATE_INFOPLIST_FILE = NO;
\t\t\t\tHEADER_SEARCH_PATHS = ("$(inherited)", "$(PROJECT_DIR)/../../core/include");
\t\t\t\tINFOPLIST_FILE = SOSVisionPro/Info.plist;
\t\t\t\tLIBRARY_SEARCH_PATHS = ("$(inherited)", "$(PROJECT_DIR)/../../build/xros-sim/core");
\t\t\t\tMARKETING_VERSION = 0.1.0;
\t\t\t\tOTHER_LDFLAGS = ("$(inherited)", "-lsos_core");
\t\t\t\tPRODUCT_BUNDLE_IDENTIFIER = com.openworld.sos.visionpro;
\t\t\t\tPRODUCT_NAME = "$(TARGET_NAME)";
\t\t\t\tSDKROOT = xros;
\t\t\t\tSWIFT_OBJC_BRIDGING_HEADER = SOSVisionPro/SOSVisionPro-Bridging-Header.h;
\t\t\t\tSWIFT_VERSION = 5.0;
\t\t\t\tTARGETED_DEVICE_FAMILY = 7;
\t\t\t\tEXCLUDED_ARCHS = x86_64;
\t\t\t\tONLY_ACTIVE_ARCH = YES;
\t\t}};
\t\t{release_cfg} /* Release */ = {{
\t\t\tisa = XCBuildConfiguration;
\t\t\tbuildSettings = {{
\t\t\t\tCODE_SIGN_STYLE = Automatic;
\t\t\t\tCURRENT_PROJECT_VERSION = 1;
\t\t\t\tGENERATE_INFOPLIST_FILE = NO;
\t\t\t\tHEADER_SEARCH_PATHS = ("$(inherited)", "$(PROJECT_DIR)/../../core/include");
\t\t\t\tINFOPLIST_FILE = SOSVisionPro/Info.plist;
\t\t\t\tLIBRARY_SEARCH_PATHS = ("$(inherited)", "$(PROJECT_DIR)/../../build/xros/core");
\t\t\t\tMARKETING_VERSION = 0.1.0;
\t\t\t\tOTHER_LDFLAGS = ("$(inherited)", "-lsos_core");
\t\t\t\tPRODUCT_BUNDLE_IDENTIFIER = com.openworld.sos.visionpro;
\t\t\t\tPRODUCT_NAME = "$(TARGET_NAME)";
\t\t\t\tSDKROOT = xros;
\t\t\t\tSWIFT_OBJC_BRIDGING_HEADER = SOSVisionPro/SOSVisionPro-Bridging-Header.h;
\t\t\t\tSWIFT_VERSION = 5.0;
\t\t\t\tTARGETED_DEVICE_FAMILY = 7;
\t\t\t\tXROS_DEPLOYMENT_TARGET = 2.0;
\t\t\t}};
\t\t\tname = Release;
\t\t}};
/* End XCBuildConfiguration section */

/* Begin XCConfigurationList section */
\t\t{config_list} /* Build configuration list */ = {{
\t\t\tisa = XCConfigurationList;
\t\t\tbuildConfigurations = (
\t\t\t\t{debug_cfg} /* Debug */,
\t\t\t\t{release_cfg} /* Release */,
\t\t\t);
\t\t\tdefaultConfigurationIsVisible = 0;
\t\t\tdefaultConfigurationName = Release;
\t\t}};
/* End XCConfigurationList section */
\t}};
\trootObject = {project_id} /* Project object */;
}}
"""

PROJ.mkdir(parents=True, exist_ok=True)
PBX.write_text(pbx)
print(f"Wrote {PBX}")
