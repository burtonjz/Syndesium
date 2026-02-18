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

#ifndef COLLECTION_WIDGET_HPP_
#define COLLECTION_WIDGET_HPP_

#include "requests/CollectionRequest.hpp"
#include "models/ComponentModel.hpp"

#include <QWidget>

class CollectionWidget: public QWidget {
    Q_OBJECT

protected:
    ComponentModel* model_ ;

    explicit CollectionWidget(ComponentModel* model, QWidget* parent = nullptr ):
        QWidget(parent),
        model_(model)
    {}

public:
    ComponentModel* getModel() const { return model_ ; }

    virtual void updateCollection(const CollectionRequest& req) = 0 ;

signals:
    void collectionEdited(const CollectionRequest& req);

};

#endif // COLLECTION_WIDGET_HPP_