/*
 *  Copyright (C) 2014 Florian Geyer <blueice@fobos.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "EntrySearcher.h"
#include "core/Group.h"

QList<Entry*> EntrySearcher::search(
	const QString &searchTerm,
	const Group* group,
	const Qt::CaseSensitivity caseSensitivity
)
{
	if(!group->isResolveSearchingEnabled())
	{
		return QList<Entry*>();
	}
	return this->searchEntries(
		searchTerm,
		group,
		caseSensitivity
	);
}

QList<Entry*> EntrySearcher::searchEntries(
	const QString &searchTerm,
	const Group* group,
	const Qt::CaseSensitivity caseSensitivity
)
{
	QList<Entry*> searchResult_;
	const QList<Entry*> &entryList_ = group->getEntries();
	for(Entry* entry_: entryList_)
	{
		searchResult_.append(
			matchEntry(
				searchTerm,
				entry_,
				caseSensitivity
			)
		);
	}
	const QList<Group*> &children_ = group->getChildren();
	for(const Group* childGroup_: children_)
	{
		if(childGroup_->isSearchingEnabled() != Group::Disable)
		{
			searchResult_.append(
				this->searchEntries(
					searchTerm,
					childGroup_,
					caseSensitivity
				)
			);
		}
	}
	return searchResult_;
}

QList<Entry*> EntrySearcher::matchEntry(
	const QString &searchTerm,
	Entry* entry,
	const Qt::CaseSensitivity caseSensitivity
) const
{
	const QStringList wordList_ = searchTerm.split(
		QRegularExpression(
			"\\s"
		),
		Qt::SkipEmptyParts
	);
	for(const QString &word_: wordList_)
	{
		if(!this->wordMatch(
			word_,
			entry,
			caseSensitivity
		))
		{
			return QList<Entry*>();
		}
	}
	return QList<Entry*>() << entry;
}

bool EntrySearcher::wordMatch(
	const QString &word,
	const Entry* entry,
	const Qt::CaseSensitivity caseSensitivity
)
{
	return entry->getTitle().contains(
		word,
		caseSensitivity
	) || entry->getUsername().contains(
		word,
		caseSensitivity
	) || entry->getURL().contains(
		word,
		caseSensitivity
	) || entry->getNotes().contains(
		word,
		caseSensitivity
	);
}
