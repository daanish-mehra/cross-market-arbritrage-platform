'use client'

import { useMemo, useState } from 'react'

interface MarketData {
  market_id: string
  market: number
  event_name: string
  best_bid: number
  best_ask: number
  bid_size: number
  ask_size: number
}

interface MarketDataViewProps {
  marketData: MarketData[]
  onChartClick?: (market: MarketData) => void
  onTradeClick?: (market: MarketData) => void
}

const marketNames: { [key: number]: string } = {
  0: 'Polymarket',
  1: 'Kalshi',
  2: 'PredictIt'
}

type SortOption = 'name' | 'spread' | 'midprice' | 'volume'

export default function MarketDataView({ marketData, onChartClick, onTradeClick }: MarketDataViewProps) {
  const [sortBy, setSortBy] = useState<SortOption>('name')
  const [searchQuery, setSearchQuery] = useState('')

  // Filter and sort markets
  const filteredAndSorted = useMemo(() => {
    if (!marketData || marketData.length === 0) {
      return []
    }

    // Markets are already unique by market_id (stored in Map in page.tsx)
    let filtered = marketData

    // Apply search filter
    if (searchQuery.trim()) {
      const query = searchQuery.toLowerCase()
      filtered = filtered.filter(data => 
        (data.event_name && data.event_name.toLowerCase().includes(query)) ||
        (data.market_id && data.market_id.toLowerCase().includes(query))
      )
    }

    // Apply sorting
    const sorted = [...filtered].sort((a, b) => {
      const midPriceA = (a.best_bid + a.best_ask) / 2
      const midPriceB = (b.best_bid + b.best_ask) / 2
      const spreadA = a.best_ask - a.best_bid
      const spreadB = b.best_ask - b.best_bid
      const volumeA = Math.min(a.bid_size || 0, a.ask_size || 0)
      const volumeB = Math.min(b.bid_size || 0, b.ask_size || 0)

      switch (sortBy) {
        case 'name':
          return (a.event_name || '').localeCompare(b.event_name || '')
        case 'spread':
          return spreadA - spreadB
        case 'midprice':
          return midPriceB - midPriceA // Highest first
        case 'volume':
          return volumeB - volumeA // Highest first
        default:
          return 0
      }
    })

    return sorted
  }, [marketData, sortBy, searchQuery])

  // Truncate long event names (smarter truncation at word boundaries)
  const truncateName = (name: string, maxLength: number = 70) => {
    if (!name || name.length <= maxLength) return name
    // Try to truncate at word boundary
    const truncated = name.substring(0, maxLength)
    const lastSpace = truncated.lastIndexOf(' ')
    if (lastSpace > maxLength * 0.7) {
      return truncated.substring(0, lastSpace) + '...'
    }
    return truncated + '...'
  }

  // Calculate probability from mid price
  const getProbability = (bid: number, ask: number) => {
    if (bid <= 0 && ask <= 0) return 50
    const midPrice = (bid + ask) / 2
    if (midPrice <= 0) return 50
    return Math.round(midPrice * 100)
  }

  // Get market symbol/abbreviation (for stocks/future use)
  const getMarketSymbol = (eventName: string) => {
    // For stocks, this would return symbols like "TSLA", "BTC", etc.
    // For now, this is a placeholder for future stock integration
    const words = eventName.split(' ')
    if (words.length >= 2) {
      return words.slice(0, 2).map(w => w.substring(0, 3).toUpperCase()).join('-')
    }
    return eventName.substring(0, 8).toUpperCase().replace(/\s/g, '-')
  }

  // Check if this is a stock (for future use - currently all are prediction markets)
  const isStock = (market: number) => {
    // Future: Add logic to detect stocks
    // For now, all markets are prediction markets (Polymarket, Kalshi, etc.)
    return false
  }

  return (
    <>
      {/* Search and Filter Bar */}
      <div className="search-filter-bar">
        <div className="search-container">
          <span className="search-icon">🔍</span>
          <input
            type="text"
            className="search-input"
            placeholder="SEARCH MARKETS..."
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
          />
        </div>
        <div className="filter-chips">
          <button
            className={`filter-chip ${sortBy === 'name' ? 'active' : ''}`}
            onClick={() => setSortBy('name')}
          >
            All Markets
          </button>
          <button
            className={`filter-chip ${sortBy === 'midprice' ? 'active' : ''}`}
            onClick={() => setSortBy('midprice')}
          >
            High Price
          </button>
          <button
            className={`filter-chip ${sortBy === 'spread' ? 'active' : ''}`}
            onClick={() => setSortBy('spread')}
          >
            Low Spread
          </button>
          <button
            className={`filter-chip ${sortBy === 'volume' ? 'active' : ''}`}
            onClick={() => setSortBy('volume')}
          >
            High Volume
          </button>
        </div>
      </div>

      {/* Market Grid */}
      <div className="market-grid">
        {filteredAndSorted.length === 0 ? (
          <div className="market-empty" style={{ gridColumn: '1 / -1', padding: '3rem' }}>
            {marketData.length === 0 ? 'No market data received yet...' : 'No markets match your search'}
          </div>
        ) : (
          filteredAndSorted.map((data) => {
            const midPrice = (data.best_bid + data.best_ask) / 2
            const probability = getProbability(data.best_bid, data.best_ask)
            const hasData = data.best_bid > 0 || data.best_ask > 0
            const isStockMarket = isStock(data.market)
            const marketSymbol = isStockMarket ? getMarketSymbol(data.event_name) : null
            
            return (
              <div key={data.market_id} className="market-card">
                <div className="market-card-header">
                  <div className="market-title-group">
                    {isStockMarket ? (
                      <>
                        <h3 title={data.event_name}>{marketSymbol}</h3>
                        <p style={{ fontSize: '11px' }} title={data.event_name}>{truncateName(data.event_name, 60)}</p>
                      </>
                    ) : (
                      <>
                        <h3 
                          className="market-title-multiline"
                          title={data.event_name || ''}
                        >
                          {data.event_name || 'Unknown Market'}
                        </h3>
                        <p style={{ fontSize: '11px', color: 'var(--text-muted)' }}>
                          {marketNames[data.market] || `Market ${data.market}`} • {data.market_id ? data.market_id.substring(0, 12) + '...' : 'N/A'}
                        </p>
                      </>
                    )}
                  </div>
                  {hasData && (
                    <div className={`trend-badge ${midPrice > 0.5 ? 'up' : 'down'}`}>
                      <span>{midPrice > 0.5 ? '▲' : '▼'}</span>
                      <span>{Math.abs((midPrice - 0.5) * 200).toFixed(1)}%</span>
                    </div>
                  )}
                </div>

                <div className="price-grid">
                  <div className="price-group">
                    <span className="price-label">Bid Price</span>
                    {data.best_bid > 0 ? (
                      <span className="price-value bid">{(data.best_bid * 100).toFixed(2)}¢</span>
                    ) : (
                      <span className="price-value" style={{ color: 'var(--text-muted)', fontSize: '14px' }}>No bids</span>
                    )}
                  </div>
                  <div className="price-group align-end">
                    <span className="price-label ask">Ask Price</span>
                    {data.best_ask > 0 ? (
                      <span className="price-value ask">{(data.best_ask * 100).toFixed(2)}¢</span>
                    ) : (
                      <span className="price-value" style={{ color: 'var(--text-muted)', fontSize: '14px' }}>No asks</span>
                    )}
                  </div>
                </div>

                {hasData && (
                  <div className="probability-section">
                    <div className="probability-header">
                      <span>PROBABILITY</span>
                      <span>{probability}% BUY</span>
                    </div>
                    <div className="probability-bar">
                      <div 
                        className="probability-bar-fill buy" 
                        style={{ width: `${probability}%` }}
                      />
                      <div 
                        className="probability-bar-fill sell" 
                        style={{ width: `${100 - probability}%` }}
                      />
                    </div>
                  </div>
                )}

                <div className="market-actions">
                  <button 
                    className="market-button primary" 
                    onClick={() => onTradeClick && onTradeClick(data)}
                  >
                    Trade
                  </button>
                  <button 
                    className="market-button icon"
                    onClick={() => onChartClick && onChartClick(data)}
                    title="View Details"
                  >
                    📊
                  </button>
                </div>
              </div>
            )
          })
        )}
      </div>
    </>
  )
}
