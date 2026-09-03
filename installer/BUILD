# installer/BUILD

genrule(
    name = "salt_msi",
    srcs = [
        "//main:salt",
        "Package.wxs",
        "Salt.wixproj",
    ],
    outs = ["salt.msi"],
    cmd_bat = """powershell -NoProfile -ExecutionPolicy Bypass -Command "$$ErrorActionPreference = 'Stop'; $$exe = (Resolve-Path '$(location //main:salt)').Path; $$proj = (Resolve-Path '$(location Salt.wixproj)').Path; $$outDir = (New-Item -ItemType Directory -Path (Join-Path $$env:TEMP ('salt_wix_' + [Guid]::NewGuid().ToString('N')))).FullName; try { & dotnet build $$proj -c Release -p:SourceExe=\\"$$exe\\" -p:OutputPath=\\"$$outDir\\\\\\"; Copy-Item (Join-Path $$outDir 'Salt.msi') '$@' } finally { Remove-Item -Recurse -Force $$outDir }" """,
    tags = [
        "no-sandbox",
        "requires-network",
    ],
)
