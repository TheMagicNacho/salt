# Salt Text Editor

This is the text editor if your salty that microsoft made notepad so bloated.

## Developer Run

### Dependencies

- msys2 : https://www.msys2.org/
- windows sdk: https://learn.microsoft.com/en-gb/windows/apps/windows-sdk/
- dotnet:
- bazel : https://bazel.build/install/windows
  - bazelisk: https://github.com/bazelbuild/bazelisk

## set up

- get teh compile_commands.json by running `bazelisk run @hedron_compile_commands//:refresh_all`

### Commands

- To run: `bazelisk run //main:salt`
- To build: `bazelisk build //main:salt`
  - Building will place the executable in: `.\bazel-bin\main\salt.exe`
- To test: `bazelisk test //tests:all`
- To build installer: `bazelisk build //installer:salt_msi`
  - Installer will be placed in: `.\bazel-bin\installer\salt.msi`
  - For an optimized build: `bazelisk build -c opt //main:salt` then `bazelisk build //installer:salt_msi`

- To generate docs: `bazelisk run //lib:docs`
  - Docs will be placed in: `.\bazel-bin\lib\docs`

- To check formatting / lint: `bazelisk build //:lint` (or `bazelisk test //:format_test`)
- To auto-format code: `bazelisk run //:format`
- To run tests (when test targets are present): `bazelisk test //tests:all`

## Release Checklist

_Replace `r1.0.0` with your target version number (must follow the `r*.*.*` pattern)._

- [ ] **1. Switch to `main` and pull the latest changes**

  ```powershell
  git checkout main
  git pull origin main
  ```

- [ ] **2. Create the release tag**

  ```powershell
  git tag -a r1.0.0 -m "Release r1.0.0"
  ```

- [ ] **3. Push the tag to GitHub (triggers the release workflow)**

  ```powershell
  git push origin r1.0.0
  ```

- [ ] **4. Verify the automated release**
  - Open **GitHub Actions** and confirm the `Main CI/CD Pipeline` succeeds.
  - Check the **Releases** page to verify `r1.0.0` is published as **Latest** with `salt.msi` attached.
  - The pipeline will handle the rest of the release by saving the msi in the release section on github.

# FAQ

- Does this have to run in windows 11?
  - Yes
- Will you ever support Mac?
  - No
- Do I have to develop in Windows 11?
  - I suppose you could cross compile, but that seems annoying.
- Why isn't this in Rust?
  - Cuz im lazy. Make your own in rust if you care so much.

# License

The Salt shaker logo comes from: https://favicon.io/emoji-favicons/salt
and is distrubuted under the CC-BY 4.0 license.
