/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_PROCESSORS_CONTEXTUAL_TEXT_EMBEDDING_TEXT_EMBEDDING_PROCESSOR_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_PROCESSORS_CONTEXTUAL_TEXT_EMBEDDING_TEXT_EMBEDDING_PROCESSOR_H_

#include <cstdint>
#include <string>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "brave/components/brave_ads/core/internal/deprecated/locale/locale_manager_observer.h"
#include "brave/components/brave_ads/core/internal/resources/resource_manager_observer.h"
#include "brave/components/brave_ads/core/internal/tabs/tab_manager_observer.h"

class GURL;

namespace brave_ads {

namespace resource {
class TextEmbedding;
}  // namespace resource

namespace processor {

class TextEmbedding final : public LocaleManagerObserver,
                            public ResourceManagerObserver,
                            public TabManagerObserver {
 public:
  explicit TextEmbedding(resource::TextEmbedding* resource);

  TextEmbedding(const TextEmbedding& other) = delete;
  TextEmbedding& operator=(const TextEmbedding& other) = delete;

  TextEmbedding(TextEmbedding&& other) noexcept = delete;
  TextEmbedding& operator=(TextEmbedding&& other) noexcept = delete;

  ~TextEmbedding() override;

  void Process(const std::string& html);

 private:
  // LocaleManagerObserver:
  void OnLocaleDidChange(const std::string& locale) override;

  // ResourceManagerObserver:
  void OnResourceDidUpdate(const std::string& id) override;

  // TabManagerObserver:
  void OnHtmlContentDidChange(int32_t tab_id,
                              const std::vector<GURL>& redirect_chain,
                              const std::string& html) override;

  const raw_ptr<resource::TextEmbedding> resource_ = nullptr;  // NOT OWNED
};

}  // namespace processor
}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_PROCESSORS_CONTEXTUAL_TEXT_EMBEDDING_TEXT_EMBEDDING_PROCESSOR_H_
