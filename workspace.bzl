# SPDX-FileCopyrightText: Copyright (c) The helly25/mbo authors (helly25.com)
# SPDX-License-Identifier: Apache-2.0
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

    http_archive(
        name = "platforms",
        urls = [
            "https://mirror.bazel.build/github.com/bazelbuild/platforms/releases/download/0.0.11/platforms-0.0.11.tar.gz",
            "https://github.com/bazelbuild/platforms/releases/download/0.0.11/platforms-0.0.11.tar.gz",
        ],
        sha256 = "29742e87275809b5e598dc2f04d86960cc7a55b3067d97221c9abbc9926bff0f",
    )

    http_archive(
        name = "rules_license",
        urls = [
            "https://mirror.bazel.build/github.com/bazelbuild/rules_license/releases/download/0.0.7/rules_license-0.0.7.tar.gz",
            "https://github.com/bazelbuild/rules_license/releases/download/0.0.7/rules_license-0.0.7.tar.gz",
        ],
        sha256 = "4531deccb913639c30e5c7512a054d5d875698daeb75d8cf90f284375fe7c360",
    )

    http_archive(
        name = "bazel_skylib",
        sha256 = "bc283cdfcd526a52c3201279cda4bc298652efa898b10b4db0837dc51652756f",
        urls = [
            "https://github.com/bazelbuild/bazel-skylib/releases/download/1.7.1/bazel-skylib-1.7.1.tar.gz",
            "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.7.1/bazel-skylib-1.7.1.tar.gz",
        ],
    )

    http_archive(
        name = "bazel_features",
        sha256 = "091d8b1e1f0bf1f7bd688b95007687e862cc489f8d9bc21c14be5fd032a8362f",
        strip_prefix = "bazel_features-1.26.0",
        url = "https://github.com/bazel-contrib/bazel_features/releases/download/v1.25.0/bazel_features-v1.26.0.tar.gz",
    )

    http_archive(
        name = "rules_foreign_cc",
        sha256 = "476303bd0f1b04cc311fc258f1708a5f6ef82d3091e53fd1977fa20383425a6a",
        strip_prefix = "rules_foreign_cc-0.10.1",
        url = "https://github.com/bazelbuild/rules_foreign_cc/releases/download/0.10.1/rules_foreign_cc-0.10.1.tar.gz",
    )

    # Used for absl/GoogleTest
    # Note GoogleTest uses "com_googlesource_code_re2" rather than "com_google_re2"
    if not native.existing_rule("com_googlesource_code_re2"):
        http_archive(
            name = "com_googlesource_code_re2",
            strip_prefix = "re2-2024-07-02",
            url = "https://github.com/google/re2/releases/download/2024-07-02/re2-2024-07-02.tar.gz",
            sha256 = "eb2df807c781601c14a260a507a5bb4509be1ee626024cb45acbd57cb9d4032b",
            repo_mapping = {"@abseil-cpp": "@com_google_absl"},
        )

    # Abseil, LTS
    # Used for GoogleTest through .bazelrc "build --define absl=1"
    github_archive(
        name = "com_google_absl",
        repo = "https://github.com/abseil/abseil-cpp",
        tag = "20250127.0",
        sha256 = "16242f394245627e508ec6bb296b433c90f8d914f73b9c026fddb905e27276e8",
    )

    # GoogleTest
    github_archive(
        name = "com_google_googletest",
        repo = "https://github.com/google/googletest",
        strip_prefix = "googletest-1.16.0",
        sha256 = "78c676fc63881529bf97bf9d45948d905a66833fbfa5318ea2cd7478cb98f399",
        tag = "v1.16.0",
        repo_mapping = {
            "@abseil-cpp": "@com_google_absl",
            "@re2": "@com_googlesource_code_re2",
        },
    )

    github_archive(
        name = "hedron_compile_commands",
        commit = "6dd21b47db481a70c61698742438230e2399b639",
        repo = "https://github.com/helly25/bazel-compile-commands-extractor",
        sha256 = "348a643defa9ab34ed9cb2ed1dc54b1c4ffef1282240aa24c457ebd8385ff2d5",
    )

    http_archive(
        name = "rules_python",
        sha256 = "2ef40fdcd797e07f0b6abda446d1d84e2d9570d234fddf8fcd2aa262da852d1c",
        strip_prefix = "rules_python-1.2.0",
        url = "https://github.com/bazelbuild/rules_python/releases/download/1.2.0/rules_python-1.2.0.tar.gz",
    )

    http_archive(
        name = "rules_java",
        url = "https://github.com/bazelbuild/rules_java/releases/download/8.9.0/rules_java-8.9.0.tar.gz",
        sha256 = "8daa0e4f800979c74387e4cd93f97e576ec6d52beab8ac94710d2931c57f8d8b",
    )

    http_archive(
        name = "com_google_protobuf",
        sha256 = "9df0e9e8ebe39f4fbbb9cf7db3d811287fe3616b2f191eb2bf5eaa12539c881f",
        strip_prefix = "protobuf-30.0",
        url = "https://github.com/protocolbuffers/protobuf/releases/download/v30.0/protobuf-30.0.tar.gz",
    )

    # Cannot yet support toolchains_llvm 1.0.0. It enables C++20 modules in a broken way.
    # Minimum requirements:
    # * 2023.10.06: https://github.com/bazel-contrib/toolchains_llvm/pull/229: Move minimum supported version to Bazel
    # * 2024.03.12: https://github.com/bazel-contrib/toolchains_llvm/pull/286: Support LLD linker for Darwin
    # * 2025.01.22: https://github.com/bazel-contrib/toolchains_llvm/pull/446: Verion 1.3.0:
    #     b3c96d2dbc698eab752366bbe747e2a7df7fa504 / sha256-ZDefpvZja99JcCg37NO4dkdH11yN2zMrx2D77sWlWug=
    # * 2025.02.15: https://github.com/bazel-contrib/toolchains_llvm/pull/461: Add LLVM 19.1.4...19.1.7:
    #     0bd3bff40ab51a8e744ccddfd24f311a9df81c2d / sha256-YpBdoaSAXSatJwLcB2yzuZw5Ls/h5+dcWip+h+pVdUo=
    # In order to go past version 1.0.0 we also add the actual fix:
    # * 2024.06.06: https://github.com/bazel-contrib/toolchains_llvm/pull/337: Force Clang modules with LLVM >= 14
    github_archive(
        name = "toolchains_llvm",
        commit = "0bd3bff40ab51a8e744ccddfd24f311a9df81c2d",
        repo = "https://github.com/bazel-contrib/toolchains_llvm",
        integrity = "sha256-YpBdoaSAXSatJwLcB2yzuZw5Ls/h5+dcWip+h+pVdUo=",
    )
    #http_archive(
    #    name = "toolchains_llvm",
    #    sha256 = "",
    #    strip_prefix = "toolchains_llvm-1.0.0",
    #    canonical_id = "1.0.0",
    #    url = "https://github.com/bazel-contrib/toolchains_llvm/releases/download/1.0.0/toolchains_llvm-1.0.0.tar.gz",
    #)
