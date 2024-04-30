#pragma once

#include <windows.h>
#include <ClibUtil/editorID.hpp>

namespace Utilities {

    const auto mod_name = static_cast<std::string>(SKSE::PluginDeclaration::GetSingleton()->GetName());
    constexpr auto po3path = "Data/SKSE/Plugins/po3_Tweaks.dll";
    bool IsPo3Installed() { return std::filesystem::exists(po3path); };

    const auto po3_err_msgbox = std::format(
        "{}: If you are trying to use Editor IDs, but you must have powerofthree's Tweaks "
        "installed. See mod page for further instructions.",
        mod_name);

    const auto general_err_msgbox = std::format("{}: Something went wrong. Please contact the mod author.", mod_name);

    const auto init_err_msgbox = std::format("{}: The mod failed to initialize and will be terminated.", mod_name);

    std::string dec2hex(unsigned int dec) {
        std::stringstream stream;
        stream << std::hex << dec;
        std::string hexString = stream.str();
        return hexString;
    };

    std::string DecodeTypeCode(std::uint32_t typeCode) {
        char buf[4];
        buf[3] = char(typeCode);
        buf[2] = char(typeCode >> 8);
        buf[1] = char(typeCode >> 16);
        buf[0] = char(typeCode >> 24);
        return std::string(buf, buf + 4);
    }

    namespace MsgBoxesNotifs {

        // https://github.com/SkyrimScripting/MessageBox/blob/ac0ea32af02766582209e784689eb0dd7d731d57/include/SkyrimScripting/MessageBox.h#L9
        class SkyrimMessageBox {
            class MessageBoxResultCallback : public RE::IMessageBoxCallback {
                std::function<void(unsigned int)> _callback;

            public:
                ~MessageBoxResultCallback() override {}
                MessageBoxResultCallback(std::function<void(unsigned int)> callback) : _callback(callback) {}
                void Run(RE::IMessageBoxCallback::Message message) override {
                    _callback(static_cast<unsigned int>(message));
                }
            };

        public:
            static void Show(const std::string& bodyText, std::vector<std::string> buttonTextValues,
                             std::function<void(unsigned int)> callback) {
                auto* factoryManager = RE::MessageDataFactoryManager::GetSingleton();
                auto* uiStringHolder = RE::InterfaceStrings::GetSingleton();
                auto* factory = factoryManager->GetCreator<RE::MessageBoxData>(
                    uiStringHolder->messageBoxData);  // "MessageBoxData" <--- can we just use this string?
                auto* messagebox = factory->Create();
                RE::BSTSmartPointer<RE::IMessageBoxCallback> messageCallback =
                    RE::make_smart<MessageBoxResultCallback>(callback);
                messagebox->callback = messageCallback;
                messagebox->bodyText = bodyText;
                for (auto& text : buttonTextValues) messagebox->buttonText.push_back(text.c_str());
                messagebox->QueueMessage();
            }
        };

        void ShowMessageBox(const std::string& bodyText, std::vector<std::string> buttonTextValues,
                            std::function<void(unsigned int)> callback) {
            SkyrimMessageBox::Show(bodyText, buttonTextValues, callback);
        }

        namespace Windows {

            int GeneralErr() {
                MessageBoxA(nullptr, general_err_msgbox.c_str(), "Error", MB_OK | MB_ICONERROR);
                return 1;
            };

            int Po3ErrMsg() {
                MessageBoxA(nullptr, po3_err_msgbox.c_str(), "Error", MB_OK | MB_ICONERROR);
                return 1;
            };
        };

        namespace InGame {

            void IniCreated() { RE::DebugMessageBox("INI created. Customize it to your liking."); };

            void InitErr() { RE::DebugMessageBox(init_err_msgbox.c_str()); };

            void GeneralErr() { RE::DebugMessageBox(general_err_msgbox.c_str()); };

            void FormTypeErr(RE::FormID id) {
                RE::DebugMessageBox(std::format("{}: The form type of the item with FormID ({}) is not supported. "
                                                "Please contact the mod author.",
                                                Utilities::mod_name, Utilities::dec2hex(id))
                                        .c_str());
            };

            void FormIDError(RE::FormID id) {
                RE::DebugMessageBox(std::format("{}: The ID ({}) could not have been found.", Utilities::mod_name,
                                                Utilities::dec2hex(id))
                                        .c_str());
            }

            void EditorIDError(std::string id) {
                RE::DebugMessageBox(
                    std::format("{}: The ID ({}) could not have been found.", Utilities::mod_name, id).c_str());
            }

            void ProblemWithContainer(std::string id) {
                RE::DebugMessageBox(
                    std::format(
                        "{}: Problem with one of the items with the form id ({}). This is expected if you have changed "
                        "the list of containers in the INI file between saves. Corresponding items will be returned to "
                        "your inventory. You can suppress this message by changing the setting in your INI.",
                        Utilities::mod_name, id)
                        .c_str());
            };

            void UninstallSuccessful() {
                RE::DebugMessageBox(
                    std::format("{}: Uninstall successful. You can now safely remove the mod.", Utilities::mod_name)
                        .c_str());
            };

            void UninstallFailed() {
                RE::DebugMessageBox(
                    std::format("{}: Uninstall failed. Please contact the mod author.", Utilities::mod_name).c_str());
            };

            void CustomErrMsg(const std::string& msg) { RE::DebugMessageBox((mod_name + ": " + msg).c_str()); };
        };
    };

    namespace Functions {

        template <typename Key, typename Value>
        bool containsValue(const std::map<Key, Value>& myMap, const Value& valueToFind) {
            for (const auto& pair : myMap) {
                if (pair.second == valueToFind) {
                    return true;
                }
            }
            return false;
        }

        std::string GetPluginVersion(const unsigned int n_stellen) {
            const auto fullVersion = SKSE::PluginDeclaration::GetSingleton()->GetVersion();
            unsigned int i = 1;
            std::string version = std::to_string(fullVersion.major());
            if (n_stellen == i) return version;
            version += "." + std::to_string(fullVersion.minor());
            if (n_stellen == ++i) return version;
            version += "." + std::to_string(fullVersion.patch());
            if (n_stellen == ++i) return version;
            version += "." + std::to_string(fullVersion.build());
            return version;
        }

        template <typename KeyType, typename ValueType>
        std::vector<KeyType> getKeys(const std::map<KeyType, ValueType>& inputMap) {
            std::vector<KeyType> keys;
            for (const auto& pair : inputMap) {
                keys.push_back(pair.first);
            }
            return keys;
        }

        namespace Vector {

            template <typename T>
            std::vector<T> mergeVectors(const std::vector<T>& vec1, const std::vector<T>& vec2) {
                std::vector<T> mergedVec;

                // Reserve enough space to avoid frequent reallocation
                mergedVec.reserve(vec1.size() + vec2.size());

                // Insert elements from vec1
                mergedVec.insert(mergedVec.end(), vec1.begin(), vec1.end());

                // Insert elements from vec2
                mergedVec.insert(mergedVec.end(), vec2.begin(), vec2.end());

                return mergedVec;
            }

            template <typename T>
            bool HasElement(const std::vector<T>& vec, const T& element) {
                return std::find(vec.begin(), vec.end(), element) != vec.end();
            }

            std::vector<int> getComplementarySet(const std::vector<int>& reference, const std::vector<int>& subset) {
                std::vector<int> complementarySet;
                for (const auto& element : reference) {
                    if (std::find(subset.begin(), subset.end(), element) == subset.end()) {
                        complementarySet.push_back(element);
                    }
                }
                return complementarySet;
            }

            std::vector<std::string> SetToVector(const std::set<std::string>& input_set) {
                // Construct a vector using the range constructor
                std::vector<std::string> result(input_set.begin(), input_set.end());
                return result;
            }

        };

        namespace String {

            template <typename T>
            std::string join(const T& container, const std::string_view& delimiter) {
                std::ostringstream oss;
                auto iter = container.begin();

                if (iter != container.end()) {
                    oss << *iter;
                    ++iter;
                }

                for (; iter != container.end(); ++iter) {
                    oss << delimiter << *iter;
                }

                return oss.str();
            }

            std::string trim(const std::string& str) {
                // Find the first non-whitespace character from the beginning
                size_t start = str.find_first_not_of(" \t\n\r");

                // If the string is all whitespace, return an empty string
                if (start == std::string::npos) return "";

                // Find the last non-whitespace character from the end
                size_t end = str.find_last_not_of(" \t\n\r");

                // Return the substring containing the trimmed characters
                return str.substr(start, end - start + 1);
            }

            std::string toLowercase(const std::string& str) {
                std::string result = str;
                std::transform(result.begin(), result.end(), result.begin(),
                               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                return result;
            }

            std::string replaceLineBreaksWithSpace(const std::string& input) {
                std::string result = input;
                std::replace(result.begin(), result.end(), '\n', ' ');
                return result;
            }

            bool includesString(const std::string& input, const std::vector<std::string>& strings) {
                std::string lowerInput = toLowercase(input);

                for (const auto& str : strings) {
                    std::string lowerStr = str;
                    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                    if (lowerInput.find(lowerStr) != std::string::npos) {
                        return true;  // The input string includes one of the strings
                    }
                }
                return false;  // None of the strings in 'strings' were found in the input string
            }

            // if it includes any of the words in the vector
            bool includesWord(const std::string& input, const std::vector<std::string>& strings) {
                std::string lowerInput = toLowercase(input);
                lowerInput = replaceLineBreaksWithSpace(lowerInput);
                lowerInput = trim(lowerInput);
                lowerInput = " " + lowerInput + " ";  // Add spaces to the beginning and end of the string

                for (const auto& str : strings) {
                    std::string lowerStr = str;
                    lowerStr = trim(lowerStr);
                    lowerStr = " " + lowerStr + " ";  // Add spaces to the beginning and end of the string
                    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                    // logger::trace("lowerInput: {} lowerStr: {}", lowerInput, lowerStr);

                    if (lowerInput.find(lowerStr) != std::string::npos) {
                        return true;  // The input string includes one of the strings
                    }
                }
                return false;  // None of the strings in 'strings' were found in the input string
            }

            std::vector<std::pair<int, bool>> encodeString(const std::string& inputString) {
                std::vector<std::pair<int, bool>> encodedValues;
                try {
                    for (int i = 0; i < 100 && inputString[i] != '\0'; i++) {
                        char ch = inputString[i];
                        if (std::isprint(ch) && (std::isalnum(ch) || std::isspace(ch) || std::ispunct(ch)) && ch >= 0 &&
                            ch <= 255) {
                            encodedValues.push_back(std::make_pair(static_cast<int>(ch), std::isupper(ch)));
                        }
                    }
                } catch (const std::exception& e) {
                    logger::error("Error encoding string: {}", e.what());
                    return encodeString("ERROR");
                }
                return encodedValues;
            }

            std::string decodeString(const std::vector<std::pair<int, bool>>& encodedValues) {
                std::string decodedString;
                for (const auto& pair : encodedValues) {
                    char ch = static_cast<char>(pair.first);
                    if (std::isalnum(ch) || std::isspace(ch) || std::ispunct(ch)) {
                        if (pair.second) {
                            decodedString += ch;
                        } else {
                            decodedString += static_cast<char>(std::tolower(ch));
                        }
                    }
                }
                return decodedString;
            }

        };

        bool isValidHexWithLength7or8(const char* input) {
            std::string inputStr(input);

            if (inputStr.substr(0, 2) == "0x") {
                // Remove "0x" from the beginning of the string
                inputStr = inputStr.substr(2);
            }

            std::regex hexRegex("^[0-9A-Fa-f]{7,8}$");  // Allow 7 to 8 characters
            bool isValid = std::regex_match(inputStr, hexRegex);
            return isValid;
        }
    };

    namespace FunctionsSkyrim {

        RE::TESForm* GetFormByID(const RE::FormID id, const std::string& editor_id = "") {
            if (!editor_id.empty()) {
                auto* form = RE::TESForm::LookupByEditorID(editor_id);
                if (form) return form;
            }
            auto form = RE::TESForm::LookupByID(id);
            if (form) return form;
            return nullptr;
        };

        template <class T = RE::TESForm>
        static T* GetFormByID(const RE::FormID id, const std::string& editor_id = "") {
            if (!editor_id.empty()) {
                auto* form = RE::TESForm::LookupByEditorID<T>(editor_id);
                if (form) return form;
            }
            T* form = RE::TESForm::LookupByID<T>(id);
            if (form) return form;
            return nullptr;
        };

        const std::string GetEditorID(const FormID a_formid) {
            if (const auto form = RE::TESForm::LookupByID(a_formid)) {
                return clib_util::editorID::get_editorID(form);
            } else {
                return "";
            }
        }

        // SkyrimThiago <3
        // https://github.com/Thiago099/DPF-Dynamic-Persistent-Forms
        namespace DynamicForm {

            const bool IsDynamicFormID(const FormID a_formID) { return a_formID >= 0xFF000000; }

            static void copyBookAppearence(RE::TESForm* source, RE::TESForm* target) {
                auto* sourceBook = source->As<RE::TESObjectBOOK>();

                auto* targetBook = target->As<RE::TESObjectBOOK>();

                if (sourceBook && targetBook) {
                    targetBook->inventoryModel = sourceBook->inventoryModel;
                }
            }

            template <class T>
            void copyComponent(RE::TESForm* from, RE::TESForm* to) {
                auto fromT = from->As<T>();

                auto toT = to->As<T>();

                if (fromT && toT) {
                    toT->CopyComponent(fromT);
                }
            }

            static void copyFormArmorModel(RE::TESForm* source, RE::TESForm* target) {
                auto* sourceModelBipedForm = source->As<RE::TESObjectARMO>();

                auto* targeteModelBipedForm = target->As<RE::TESObjectARMO>();

                if (sourceModelBipedForm && targeteModelBipedForm) {
                    logger::info("armor");

                    targeteModelBipedForm->armorAddons = sourceModelBipedForm->armorAddons;
                }
            }

            static void copyFormObjectWeaponModel(RE::TESForm* source, RE::TESForm* target) {
                auto* sourceModelWeapon = source->As<RE::TESObjectWEAP>();

                auto* targeteModelWeapon = target->As<RE::TESObjectWEAP>();

                if (sourceModelWeapon && targeteModelWeapon) {
                    logger::info("weapon");

                    targeteModelWeapon->firstPersonModelObject = sourceModelWeapon->firstPersonModelObject;

                    targeteModelWeapon->attackSound = sourceModelWeapon->attackSound;

                    targeteModelWeapon->attackSound2D = sourceModelWeapon->attackSound2D;

                    targeteModelWeapon->attackSound = sourceModelWeapon->attackSound;

                    targeteModelWeapon->attackFailSound = sourceModelWeapon->attackFailSound;

                    targeteModelWeapon->idleSound = sourceModelWeapon->idleSound;

                    targeteModelWeapon->equipSound = sourceModelWeapon->equipSound;

                    targeteModelWeapon->unequipSound = sourceModelWeapon->unequipSound;

                    targeteModelWeapon->soundLevel = sourceModelWeapon->soundLevel;
                }
            }

            static void copyMagicEffect(RE::TESForm* source, RE::TESForm* target) {
                auto* sourceEffect = source->As<RE::EffectSetting>();

                auto* targetEffect = target->As<RE::EffectSetting>();

                if (sourceEffect && targetEffect) {
                    targetEffect->effectSounds = sourceEffect->effectSounds;

                    targetEffect->data.castingArt = sourceEffect->data.castingArt;

                    targetEffect->data.light = sourceEffect->data.light;

                    targetEffect->data.hitEffectArt = sourceEffect->data.hitEffectArt;

                    targetEffect->data.effectShader = sourceEffect->data.effectShader;

                    targetEffect->data.hitVisuals = sourceEffect->data.hitVisuals;

                    targetEffect->data.enchantShader = sourceEffect->data.enchantShader;

                    targetEffect->data.enchantEffectArt = sourceEffect->data.enchantEffectArt;

                    targetEffect->data.enchantVisuals = sourceEffect->data.enchantVisuals;

                    targetEffect->data.projectileBase = sourceEffect->data.projectileBase;

                    targetEffect->data.explosion = sourceEffect->data.explosion;

                    targetEffect->data.impactDataSet = sourceEffect->data.impactDataSet;

                    targetEffect->data.imageSpaceMod = sourceEffect->data.imageSpaceMod;
                }
            }

            void copyAppearence(RE::TESForm* source, RE::TESForm* target) {
                copyFormArmorModel(source, target);

                copyFormObjectWeaponModel(source, target);

                copyMagicEffect(source, target);

                copyBookAppearence(source, target);

                copyComponent<RE::BGSPickupPutdownSounds>(source, target);

                copyComponent<RE::BGSMenuDisplayObject>(source, target);

                copyComponent<RE::TESModel>(source, target);

                copyComponent<RE::TESBipedModelForm>(source, target);
            }

        };

    };

    namespace Types {

        struct DFSaveData {
            FormID dyn_formid = 0;
            std::pair<bool, uint32_t> custom_id = {false, 0};
            float acteff_elapsed = -1.f;
        };
        using DFSaveDataLHS = std::pair<FormID, std::string>;
        using DFSaveDataRHS = std::vector<DFSaveData>;

    };


    bool read_string(SKSE::SerializationInterface* a_intfc, std::string& a_str) {
        std::vector<std::pair<int, bool>> encodedStr;
        std::size_t size;
        if (!a_intfc->ReadRecordData(size)) {
            return false;
        }
        for (std::size_t i = 0; i < size; i++) {
            std::pair<int, bool> temp_pair;
            if (!a_intfc->ReadRecordData(temp_pair)) {
                return false;
            }
            encodedStr.push_back(temp_pair);
        }
        a_str = Functions::String::decodeString(encodedStr);
        return true;
    }

    bool write_string(SKSE::SerializationInterface* a_intfc, const std::string& a_str) {
        const auto encodedStr = Functions::String::encodeString(a_str);
        // i first need the size to know no of iterations
        const auto size = encodedStr.size();
        if (!a_intfc->WriteRecordData(size)) {
            return false;
        }
        for (const auto& temp_pair : encodedStr) {
            if (!a_intfc->WriteRecordData(temp_pair)) {
                return false;
            }
        }
        return true;
    }

    // github.com/ozooma10/OSLAroused/blob/29ac62f220fadc63c829f6933e04be429d4f96b0/src/PersistedData.cpp
    template <typename T, typename U>
    // BaseData is based off how powerof3's did it in Afterlife
    class BaseData {
    public:
        float GetData(T formId, T missing) {
            Locker locker(m_Lock);
            if (auto idx = m_Data.find(formId) != m_Data.end()) {
                return m_Data[formId];
            }
            return missing;
        }

        void SetData(T formId, U value) {
            Locker locker(m_Lock);
            m_Data[formId] = value;
        }

        virtual const char* GetType() = 0;

        virtual bool Save(SKSE::SerializationInterface*, std::uint32_t, std::uint32_t) { return false; };
        virtual bool Save(SKSE::SerializationInterface*) { return false; };
        virtual bool Load(SKSE::SerializationInterface*) { return false; };

        void Clear() {
            Locker locker(m_Lock);
            m_Data.clear();
        };

        virtual void DumpToLog() = 0;

    protected:
        std::map<T, U> m_Data;

        using Lock = std::recursive_mutex;
        using Locker = std::lock_guard<Lock>;
        mutable Lock m_Lock;
    };

    class DFSaveLoadData : public BaseData<Types::DFSaveDataLHS, Types::DFSaveDataRHS> {
    public:
        void DumpToLog() override {
            // nothing for now
        }

        [[nodiscard]] bool Save(SKSE::SerializationInterface* serializationInterface) override {
            assert(serializationInterface);
            Locker locker(m_Lock);

            const auto numRecords = m_Data.size();
            if (!serializationInterface->WriteRecordData(numRecords)) {
                logger::error("Failed to save {} data records", numRecords);
                return false;
            }

            for (const auto& [lhs, rhs] : m_Data) {
                // we serialize formid, editorid, and refid separately
                std::uint32_t formid = lhs.first;
                logger::trace("Formid:{}", formid);
                if (!serializationInterface->WriteRecordData(formid)) {
                    logger::error("Failed to save formid");
                    return false;
                }

                const std::string editorid = lhs.second;
                logger::trace("Editorid:{}", editorid);
                write_string(serializationInterface, editorid);

                // save the number of rhs records
                const auto numRhsRecords = rhs.size();
                if (!serializationInterface->WriteRecordData(numRhsRecords)) {
                    logger::error("Failed to save the size {} of rhs records", numRhsRecords);
                    return false;
                }

                for (const auto& rhs_ : rhs) {
                    logger::trace("size of rhs_: {}", sizeof(rhs_));
                    if (!serializationInterface->WriteRecordData(rhs_)) {
                        logger::error("Failed to save data");
                        return false;
                    }
                }
            }
            return true;
        }

        [[nodiscard]] bool Save(SKSE::SerializationInterface* serializationInterface, std::uint32_t type,
                                std::uint32_t version) override {
            if (!serializationInterface->OpenRecord(type, version)) {
                logger::error("Failed to open record for Data Serialization!");
                return false;
            }

            return Save(serializationInterface);
        }

        [[nodiscard]] bool Load(SKSE::SerializationInterface* serializationInterface) override {
            assert(serializationInterface);

            std::size_t recordDataSize;
            serializationInterface->ReadRecordData(recordDataSize);
            logger::info("Loading data from serialization interface with size: {}", recordDataSize);

            Locker locker(m_Lock);
            m_Data.clear();

            logger::trace("Loading data from serialization interface.");
            for (auto i = 0; i < recordDataSize; i++) {
                Types::DFSaveDataRHS rhs;

                std::uint32_t formid = 0;
                logger::trace("ReadRecordData:{}", serializationInterface->ReadRecordData(formid));
                if (!serializationInterface->ResolveFormID(formid, formid)) {
                    logger::error("Failed to resolve form ID, 0x{:X}.", formid);
                    continue;
                }

                std::string editorid;
                if (!read_string(serializationInterface, editorid)) {
                    logger::error("Failed to read editorid");
                    return false;
                }

                logger::trace("Formid:{}", formid);
                logger::trace("Editorid:{}", editorid);

                Types::DFSaveDataLHS lhs({formid, editorid});
                logger::trace("Reading value...");

                std::size_t rhsSize = 0;
                logger::trace("ReadRecordData: {}", serializationInterface->ReadRecordData(rhsSize));
                logger::trace("rhsSize: {}", rhsSize);

                for (auto j = 0; j < rhsSize; j++) {
                    Types::DFSaveData rhs_;
                    logger::trace("ReadRecordData: {}", serializationInterface->ReadRecordData(rhs_));
                    logger::trace(
                        "rhs_ content: dyn_formid: {}, customid_bool: {},"
                        "customid: {}, acteff_elapsed: {}",
                        rhs_.dyn_formid, rhs_.custom_id.first, rhs_.custom_id.second, rhs_.acteff_elapsed);
                    rhs.push_back(rhs_);
                }

                m_Data[lhs] = rhs;
                logger::info("Loaded data for formid {}, editorid {}", formid, editorid);
            }

            return true;
        }
    };
};
