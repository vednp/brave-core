/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_PROCESSORS_CONTEXTUAL_TEXT_CLASSIFICATION_TEXT_CLASSIFICATION_PROCESSOR_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_PROCESSORS_CONTEXTUAL_TEXT_CLASSIFICATION_TEXT_CLASSIFICATION_PROCESSOR_H_

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
class TextClassification;
}  // namespace resource

namespace processor {

class TextClassification final : public LocaleManagerObserver,
                                 public ResourceManagerObserver,
                                 public TabManagerObserver {
 public:
  explicit TextClassification(resource::TextClassification* resource);

  TextClassification(const TextClassification& other) = delete;
  TextClassification& operator=(const TextClassification& other) = delete;

  TextClassification(TextClassification&& other) noexcept = delete;
  TextClassification& operator=(TextClassification&& other) noexcept = delete;

  ~TextClassification() override;

  void Process(const std::string& text);

 private:
  // LocaleManagerObserver:
  void OnLocaleDidChange(const std::string& locale) override;

  // ResourceManagerObserver:
  void OnResourceDidUpdate(const std::string& id) override;

  // TabManagerObserver:
  void OnTextContentDidChange(int32_t tab_id,
                              const std::vector<GURL>& redirect_chain,
                              const std::string& content) override;

  const raw_ptr<resource::TextClassification> resource_ = nullptr;  // NOT OWNED
};

}  // namespace processor
}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_PROCESSORS_CONTEXTUAL_TEXT_CLASSIFICATION_TEXT_CLASSIFICATION_PROCESSOR_H_
