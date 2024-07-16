# Copyright 2023 M.Boerger
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Loads modules required by the workspace."""

load("//bzl:archive.bzl", "github_archive", "http_archive")

# buildifier: disable=unnamed-macro
def workspace_load_modules():
    """Loads all modules requred by the workspace."""

    # https://github.com/bazelbuild/platforms
    http_archive(
        name = "platforms",
        urls = [
            "https://github.com/bazelbuild/platforms/releases/download/0.0.10/platforms-0.0.10.tar.gz",
            "https://mirror.bazel.build/github.com/bazelbuild/platforms/releases/download/0.0.10/platforms-0.0.10.tar.gz",
        ],
        sha256 = "218efe8ee736d26a3572663b374a253c012b716d8af0c07e842e82f238a0a7ee",
    )

    # https://github.com/bazelbuild/rules_license
    http_archive(
        name = "rules_license",
        urls = [
            "https://github.com/bazelbuild/rules_license/releases/download/0.0.8/rules_license-0.0.8.tar.gz",
            "https://mirror.bazel.build/github.com/bazelbuild/rules_license/releases/download/0.0.8/rules_license-0.0.8.tar.gz",
        ],
        sha256 = "241b06f3097fd186ff468832150d6cc142247dc42a32aaefb56d0099895fd229",
    )

    # https://github.com/bazelbuild/bazel-skylib
    http_archive(
        name = "bazel_skylib",
        sha256 = "bc283cdfcd526a52c3201279cda4bc298652efa898b10b4db0837dc51652756f",
        urls = [
            "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.7.1/bazel-skylib-1.7.1.tar.gz",
            "https://github.com/bazelbuild/bazel-skylib/releases/download/1.7.1/bazel-skylib-1.7.1.tar.gz",
        ],
    )

    # https://github.com/bazelbuild/rules_foreign_cc
    http_archive(
        name = "rules_foreign_cc",
        sha256 = "4b33d62cf109bcccf286b30ed7121129cc34cf4f4ed9d8a11f38d9108f40ba74",
        strip_prefix = "rules_foreign_cc-0.11.1",
        url = "https://github.com/bazelbuild/rules_foreign_cc/releases/download/0.11.1/rules_foreign_cc-0.11.1.tar.gz",
    )

    # Abseil, LTS 20230125
    # Used for GoogleTest through .bazelrc "build --define absl=1"
    github_archive(
        name = "com_google_absl",
        repo = "https://github.com/abseil/abseil-cpp",
        sha256 = "733726b8c3a6d39a4120d7e45ea8b41a434cdacde401cba500f14236c49b39dc",
        strip_prefix = "abseil-cpp-20240116.2",
        tag = "20240116.2",
    )

    # Used for absl/GoogleTest
    # Note GoogleTest uses "com_googlesource_code_re2" rather than "com_google_re2"
    github_archive(
        name = "com_googlesource_code_re2",
        repo_mapping = {"@abseil-cpp": "@com_google_absl"},
        repo = "https://github.com/google/re2",
        sha256 = "eb2df807c781601c14a260a507a5bb4509be1ee626024cb45acbd57cb9d4032b",
        strip_prefix = "re2-2024-07-02",
        tag = "2024-07-02",
    )

    # GoogleTest
    github_archive(
        name = "com_google_googletest",
        repo = "https://github.com/google/googletest",
        sha256 = "7315acb6bf10e99f332c8a43f00d5fbb1ee6ca48c52f6b936991b216c586aaad",
        strip_prefix = "googletest-1.15.0",
        tag = "v1.15.0",
    )

    # Needed because protobuf_deps brings rules_python 0.26.0 which is broken:
    # https://github.com/bazelbuild/rules_python/issues/1543
    http_archive(
        name = "rules_python",
        sha256 = "5868e73107a8e85d8f323806e60cad7283f34b32163ea6ff1020cf27abef6036",
        strip_prefix = "rules_python-0.25.0",
        url = "https://github.com/bazelbuild/rules_python/releases/download/0.25.0/rules_python-0.25.0.tar.gz",
    )

    github_archive(
        name = "com_google_protobuf",
        repo = "https://github.com/protocolbuffers/protobuf",
        sha256 = "e4ff2aeb767da6f4f52485c2e72468960ddfe5262483879ef6ad552e52757a77",
        strip_prefix = "protobuf-27.2",
        tag = "v27.2",
    )

    # hedron_compile_commands: Used to generate `compile_commands.json` from Bazel.
    # > bazel run @hedron_compile_commands//:refresh_all
    github_archive(
        name = "hedron_compile_commands",
        commit = "daae6f40adfa5fdb7c89684cbe4d88b691c63b2d",
        repo = "https://github.com/helly25/bazel-compile-commands-extractor",
        sha256 = "43451a32bf271e7ba4635a07f7996d535501f066c0fe8feab04fb0c91dd5986e",
    )

    # Cannot yet support toolchains_llvm 1.0.0. It enables C++20 modules in a broken way.
    # Minimum requirements:
    # * 2023.10.06: https://github.com/bazel-contrib/toolchains_llvm/pull/229: Move minimum supported version to Bazel
    # * 2024.03.12: https://github.com/bazel-contrib/toolchains_llvm/pull/286: Support LLD linker for Darwin
    # In order to go past version 1.0.0 we also add the actual fix:
    # * 2024.06.06: https://github.com/bazel-contrib/toolchains_llvm/pull/337: Force Clang modules with LLVM >= 14q
    github_archive(
        name = "toolchains_llvm",
        commit = "b193952cdb9dd3134f51159d820614ff32dba079",  # https://github.com/bazel-contrib/toolchains_llvm/pull/337
        repo = "https://github.com/bazel-contrib/toolchains_llvm",
        integrity = "sha256-eqCfrC/cwTsdE2+DkJQKQmxtlM8mb/nNaZSdg2H8dbk=",
    )
