/*
 * Copyright (C) 2026 Jared Burton
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "util/util.hpp"

QJsonObject Util::nlohmannToQJsonObject(nlohmann::json j){
    QString jsonStr = QString::fromStdString(j.dump());
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    return doc.object() ;
}

nlohmann::json Util::QJsonObjectToNlohmann(QJsonObject obj){
    QJsonDocument doc(obj);
    std::string s = doc.toJson(QJsonDocument::Compact).toStdString();
    return nlohmann::json::parse(s);
}