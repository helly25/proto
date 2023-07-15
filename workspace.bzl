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

    # 0.0.6, 2022-08-26
    http_archive(
        name = "platforms",
        urls = [
            "https://mirror.bazel.build/github.com/bazelbuild/platforms/releases/download/0.0.6/platforms-0.0.6.tar.gz",
            "https://github.com/bazelbuild/platforms/releases/download/0.0.6/platforms-0.0.6.tar.gz",
        ],
        sha256 = "5308fc1d8865406a49427ba24a9ab53087f17f5266a7aabbfc28823f3916e1ca",
    )
    # 0.0.7
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
        sha256 = "1c531376ac7e5a180e0237938a2536de0c54d93f5c278634818e0efc952dd56c",
        urls = [
            "https://github.com/bazelbuild/bazel-skylib/releases/download/1.0.3/bazel-skylib-1.0.3.tar.gz",
            "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.0.3/bazel-skylib-1.0.3.tar.gz",
        ],
    )

    # Used for absl/GoogleTest
    # Note GoogleTest uses "com_googlesource_code_re2" rather than "com_google_re2"
    http_archive(
        name = "com_googlesource_code_re2",
        strip_prefix = "re2-2023-06-02",
        urls = ["https://github.com/google/re2/archive/refs/tags/2023-06-02.tar.gz"],
        sha256 = "4ccdd5aafaa1bcc24181e6dd3581c3eee0354734bb9f3cb4306273ffa434b94f",
    )

    # Abseil, LTS 20230125
    # Used for GoogleTest through .bazelrc "build --define absl=1"
    github_archive(
        name = "com_google_absl",
        repo = "https://github.com/abseil/abseil-cpp",
        tag = "20230125.3",
        sha256 = "5366d7e7fa7ba0d915014d387b66d0d002c03236448e1ba9ef98122c13b35c36",
    )

    # GoogleTest
    github_archive(
        name = "com_google_googletest",
        repo = "https://github.com/google/googletest",
        strip_prefix = "googletest-1.13.0",
        tag = "v1.13.0",
        sha256 = "ad7fdba11ea011c1d925b3289cf4af2c66a352e18d4c7264392fead75e919363",
    )

    github_archive(
        name = "com_google_protobuf",
        repo = "https://github.com/protocolbuffers/protobuf",
        strip_prefix = "protobuf-23.4",
        tag = "v23.4",
        sha256 = "a700a49470d301f1190a487a923b5095bf60f08f4ae4cac9f5f7c36883d17971",
    )

